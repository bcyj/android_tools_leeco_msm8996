/* =======================================================================
                              FileSourcehelper.cpp
DESCRIPTION
  Definiton of interfaces for Filesource Module.

  The Intent of having this module is to Provide a common interface for
  upperlayers to control the FileBase (for both MP4 and ASF file formats)
  and perform basic operations like Play/Pause/Open/Seek based on the TrackId
  independent of the media (Audio/Video/Text) type.

Copyright (c) 2008-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential..

========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileSource/main/latest/src/filesourcehelper.cpp#161 $$
$DateTime: 2014/05/13 22:50:58 $
$Change: 5885284 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "AEEStdDef.h"
#include "filebase.h"
#include "filesourcehelper.h"
#include "zrex_string.h"
#include "MMTimer.h"
#include "MMSignal.h"
#include "MMThread.h"
#include "MMDebugMsg.h"
#ifdef FEATURE_FILESOURCE_AVI
#include "avifile.h"
  #ifdef FEATURE_FILESOURCE_DIVX_DRM
    #include "Qtv_DivxRegistration.h"
  #endif
#endif
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
#include "DataSourcePort.h"
#endif
#ifdef FEATURE_FILESOURCE_DRM_DCF
 #include "ixipc.h"
 #include "IxStream.h"
#endif
#include "MMCriticalSection.h"
/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#ifdef FEATURE_FILESOURCE_3GP_PARSER
  #define VIDEO_FMT_STREAM_AUDIO_EVRC_PV 4
#endif
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
 inline bool IS_FILESOURCE_STATUS_OK(uint8 nstatus)
 {
   if(nstatus == FS_STATE_READY)
   {
     return true;
   }
   return false;
 }

 const unsigned int FileSourceHelper::SOURCE_THREAD_STACK_SIZE = 16384;
 const uint32 FileSourceHelper::OPEN_FILE_EVENT = 0;
 const uint32 FileSourceHelper::THREAD_EXIT_EVENT = 1;
 const uint32 FileSourceHelper::CLOSE_FILE_EVENT = 2;
 const uint32 FileSourceHelper::SEEK_FILE_EVENT = 3;
 const uint32 FileSourceHelper::CLOSE_FILE_DONE_EVENT = 4;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

 /* Constructor for FileSource when called with a function call back
 */
FileSourceHelper::FileSourceHelper(FileSourceHelperCallbackFuncType callBack,
                                   void * pClientData, bool bAsync) :
m_pIStreamPort(NULL),
m_pIxStream(NULL),
m_fileSourceHelperCallBackFunc(NULL),
m_nCurrentPlaybacktimeForSeek(0),
m_nNumSync(0),
m_nSeekAbsoluteTime(0),
m_bSeekToSync(true),
m_nPlaybackTime(-1),
m_nTrackIdToSeek(-1),
m_eSeekType(FILE_SOURCE_SEEK_UKNOWN),
m_bOpenPending(false),
m_bClosePending(false),
m_bEveryThingOK(true),
m_audioFilename(),
m_videoFilename(),
m_textFilename(),
m_pAudioBuffer(NULL),
m_pVideoBuffer(NULL),
m_pTextBuffer(NULL),
m_nAudioBufSize(0),
m_nVideoBufSize(0),
m_nTextBufSize(0),
m_pSignalQ(NULL),
m_pCloseDoneSignalQ(NULL),
m_pOpenFileSignal(NULL),
m_pCloseFileSignal(NULL),
m_pCloseFileDoneSignal(NULL),
m_pSeekFileSignal(NULL),
m_pExitSignal(NULL),
m_pSourceThreadHandle(NULL),
m_pCriticalSection(NULL),
m_hFileFormatRequested(FILE_SOURCE_UNKNOWN),
m_nTrackIdInfoIndexToUse(0),
m_eState(FS_STATE_IDLE),
m_pAudioCriticalSection(NULL),
m_pVideoCriticalSection(NULL),
m_pTextCriticalSection(NULL),
m_pAudioDataBuffer(NULL)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::FileSource");

  // Initialize FileSource data structures
  BaseInitData();

  // Assign callBack function from client user
  m_fileSourceHelperCallBackFunc = callBack;
  m_pClientData = pClientData;
  m_bFSAsync = bAsync;

  if(!m_fileSourceHelperCallBackFunc)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "FileSource::FileSource m_fileSourceHelperCallBackFunc is NULL!!!!");
    m_bEveryThingOK = false;
  }

  //! Mark below flag as false and so that thread will not be created
  if (false == m_bFSAsync)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"FileSource:: synchronous");
    m_bEveryThingOK = false;
  }


  /* Create the signal Q for the thread to wait on. */
  if( m_bEveryThingOK && ( 0 != MM_SignalQ_Create( &m_pSignalQ ) ) )
  {
    m_bEveryThingOK = false;
  }
  /* create the signal Q to block in caller's context in CloseFile API*/
  if( (m_bEveryThingOK) && ( 0 != MM_SignalQ_Create( &m_pCloseDoneSignalQ ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create the open file signal on signalQ.
  *  This signal is set when OpenFile gets called.
  *  OpenFile will set m_pOpenFileSignal for file source thread.
  */
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Signal_Create( m_pSignalQ,
                                   (void *) &OPEN_FILE_EVENT,
                                   NULL,
                                   &m_pOpenFileSignal ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create the thread exit signal on signalQ.
  *  This signal is set when FileSource thread needs to exit.
  */
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Signal_Create( m_pSignalQ,
                                   (void *) &THREAD_EXIT_EVENT,
                                   NULL,
                                   &m_pExitSignal ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create the close file signal on signalQ.
  *  This signal is set when CloseFile gets called.
  *  CloseFile will set m_pCloseFileSignal for file source thread.
  */
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Signal_Create( m_pSignalQ,
                                   (void *) &CLOSE_FILE_EVENT,
                                   NULL,
                                   &m_pCloseFileSignal ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create the close file done signal on CloseDoneSignalQ.
  *  This signal is set when FileSource thread
  *  processes Close file signal.
  */
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Signal_Create( m_pCloseDoneSignalQ,
                                   (void *) &CLOSE_FILE_DONE_EVENT,
                                   NULL,
                                   &m_pCloseFileDoneSignal ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create the seek file signal on signalQ.
  *  This signal is set when
  *  SeekAbsolutePosition/SeekRelativeSyncPoint gets called.
  */
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Signal_Create( m_pSignalQ,
                                   (void *) &SEEK_FILE_EVENT,
                                   NULL,
                                   &m_pSeekFileSignal ) ) )
  {
    m_bEveryThingOK = false;
  }

  /* Create and start the FileSource Thread*/
  if ( ( m_bEveryThingOK ) && ( 0 != MM_Thread_CreateEx( MM_Thread_DefaultPriority,
                                   0,
                                   SourceThreadEntry,
                                   (void *) this,
                                   SOURCE_THREAD_STACK_SIZE,
                                   (char *)"FILESOURCE",
                                   &m_pSourceThreadHandle ) ) )
  {
    m_bEveryThingOK = false;
  }
  //! reset everything OK flag to true
  if (false == m_bFSAsync)
  {
     m_bEveryThingOK = true;
  }

  if(m_bEveryThingOK)
  {
    m_eState = FS_STATE_INIT;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::FileSource m_bEveryThingOK %d",m_bEveryThingOK);
}

/* Destructor for FileSource
 */
FileSourceHelper::~FileSourceHelper()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::~FileSource m_bEveryThingOK %d",m_bEveryThingOK);
  /* Thread exit code. */
  int exitCode = 0;

  //Check to see if thread was started successfully before posting any signal.
  if(m_bEveryThingOK && m_bFSAsync)
  {
    /* Send the exit signal to the source thread. */
    MM_Signal_Set( m_pExitSignal );

    MM_Thread_Join( m_pSourceThreadHandle, &exitCode );
  }

  /* Release the thread resources. */
  if(m_pSourceThreadHandle)
  {
    MM_Thread_Release( m_pSourceThreadHandle );
  }

  /* Release the open file signal. */
  if(m_pOpenFileSignal)
  {
    MM_Signal_Release( m_pOpenFileSignal );
  }

  /* Release the close file signal. */
  if(m_pCloseFileSignal)
  {
    MM_Signal_Release( m_pCloseFileSignal );
  }

  /* Release the close done signal. */
  if(m_pCloseFileDoneSignal)
  {
    MM_Signal_Release( m_pCloseFileDoneSignal );
  }

  /* Release the seek file signal. */
  if(m_pSeekFileSignal)
  {
    MM_Signal_Release( m_pSeekFileSignal );
  }

  /* Release the exit signal. */
  if(m_pExitSignal)
  {
    MM_Signal_Release( m_pExitSignal );
  }

  /* Release all signal Qs. */
  if(m_pSignalQ)
  {
    MM_SignalQ_Release( m_pSignalQ );
  }
  if(m_pCloseDoneSignalQ)
  {
    MM_SignalQ_Release( m_pCloseDoneSignalQ );
  }

  if(m_pAudioDataBuffer)
  {
    if(m_pAudioDataBuffer->pDataBuff)
    {
      MM_Free(m_pAudioDataBuffer->pDataBuff);
    }
    MM_Free(m_pAudioDataBuffer);
    m_pAudioDataBuffer = NULL;
  }
  DestroyMediaHandles();
}

/*! ======================================================================
    @brief  Open a File.

    @detail This method is called only in the case of Local file playback.
            FileSource  Module will pass the filenames to lower layer (fileBase) to open
            the files and get information about the tracks, headers etc.

    @param[in]  audioFilename  name of the audio file with audio track.
    @param[in]  videoFilename  name of the video file with video track.
    @param[in]  textFilename   name of the text file with text track.
    @param[in]  format         If file format is already known,caller
                               can pass in the information into FileSource.
                               When format != FILE_SOURCE_UNKNOWN,FileSource
                               will instantiate the given parser without doing any
                               file format check.
    @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
    @return     filesource status.

    @note   It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one filename is passed.
========================================================================== */

FileSourceStatus  FileSourceHelper::OpenFile(wchar_t* audioFilename,
                                             wchar_t* videoFilename,
                                             wchar_t* textFilename,
                                             FileSourceFileFormat format,
                                             bool     blookforcodecconfig)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile wchar_t");
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  if((m_bEveryThingOK) && (FS_STATE_INIT == m_eState) )
  {
    m_audioFilename = audioFilename;
    m_videoFilename = videoFilename;
    m_textFilename = textFilename;
    m_bLookForCodecHdr = blookforcodecconfig;
    status = FILE_SOURCE_SUCCESS;
    m_hFileFormatRequested = format;

    if (m_bFSAsync)
    {
      /* Post the event to the file open handler */
      MM_Signal_Set( m_pOpenFileSignal );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::SourceThread received OPEN_FILE_EVENT");
      HandleOpenFileCommand();
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::OpenFile FS is not in valid state %d", m_eState);
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "FileSource::OpenFile wchar_t status %d",status);
  return status;
}

/*! ======================================================================
   @brief  Buffer playback.

    @detail This method is called to play audio/video/text from buffers passed in.
            It's possible to play audio/video/text from separate buffers or user
            can just pass in the same buffer for audio/video/text.
            FileSource  Module will pass the buffers to lower layer (fileBase) to parse
            and get information about the tracks, headers etc.

    @param[in]  pAudioBuf     Buffer to play audio.
    @param[in]  nAudioBufSize Total size of audio buffer
    @param[in]  pVideoBuf     Buffer to play video.
    @param[in]  nAudioBufSize Total size of video buffer
    @param[in]  pTextBuf      Buffer to play text.
    @param[in]  nTextBufSize  Total size of text buffer
    @param[in]  format        If file format is already known,caller
                              can pass in the information into FileSource.
                              When format != FILE_SOURCE_UNKNOWN,FileSource
                              will instantiate the given parser without doing any
                              file format check.
    @return     filesource status.

    @note   It is assumed that these buffers will be valid through out the playback
            and data bytes are stored at contiguous address. FileSource module
            can request the data within the valid offsets range at anytime.

            0 - nAudioBufSize
            0 - nVideoBufSize
            0 - nTextBufSize

            It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one buffer is not NULL.
========================================================================== */
FileSourceStatus  FileSourceHelper::OpenFile(unsigned char* pAudioBuf,
                                             uint32         nAudioBufSize,
                                             unsigned char* pVideoBuf,
                                             uint32         nVideoBufSize,
                                             unsigned char* pTextBuf,
                                             uint32         nTextBufSize,
                                             FileSourceFileFormat format)
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile unsigned char*");
  if((m_bEveryThingOK) && (FS_STATE_INIT == m_eState) )
  {
    m_pAudioBuffer = pAudioBuf;
    m_pVideoBuffer = pVideoBuf;
    m_pTextBuffer  = pTextBuf;
    m_nAudioBufSize = nAudioBufSize;
    m_nVideoBufSize = nVideoBufSize;
    m_nTextBufSize = nTextBufSize;
    status = FILE_SOURCE_SUCCESS;
    m_hFileFormatRequested = format;
    if (m_bFSAsync)
    {
      /* Post the event to the file open handler */
      MM_Signal_Set( m_pOpenFileSignal );
    }
    else
    {
      HandleOpenFileCommand();
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::OpenFile FS is not in valid state %d", m_eState);
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "FileSource::OpenFile unsigned char* status %d", status);
  return status;
}

/*! ======================================================================
   @brief   Opens a Streaming file

   @detail  It takes iStreamPort as input parameter. iStreamPort will provide interfaces
            to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream   Input Stream of Audio/video/text tracks
   @param[in]  format         If file format is already known,caller
                              can pass in the information into FileSource.
                              When format != FILE_SOURCE_UNKNOWN,FileSource
                              will instantiate the given parser without doing any
                              file format check.
   @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
========================================================================== */

FileSourceStatus FileSourceHelper::OpenFile(video::iStreamPort* pInputStream,
                                            FileSourceFileFormat format,
                                            bool     blookforcodecconfig)
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "FileSource::OpenFile iStreamPort Cur State %d", m_eState);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if((m_bEveryThingOK) && (FS_STATE_INIT == m_eState) )
  {
    m_bLookForCodecHdr = blookforcodecconfig;
    m_pIStreamPort = pInputStream;
    m_hFileFormatRequested = format;
    if (m_bFSAsync)
    {
      /* Post the event to the file open handler */
      MM_Signal_Set( m_pOpenFileSignal );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::SourceThread received OPEN_FILE_EVENT");
      HandleOpenFileCommand();
    }
    status = FILE_SOURCE_SUCCESS;
  }
  else if (FS_STATE_READY == m_eState)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "FileSource::SourceThread received OPEN_FILE_EVENT");
    HandleOpenFileCommand();
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::OpenFile FS is not in valid state %d", m_eState);
  }
#else
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
           "FileSource::FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD not enabled");
#endif/*FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD*/

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FileSource::OpenFile iStreamPort status %d",status);
  return status;
}

/*! ======================================================================
   @brief    Opens IxStream for DCF playback.

   @detail  It takes IxStream* as input parameter. dcf_ixstream_type will provide
            interfaces to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream   Input DCF Stream of Audio/video/text tracks
   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DRM_DCF
FileSourceStatus FileSourceHelper::OpenFile( IxStream* pInputStream)
#else
FileSourceStatus FileSourceHelper::OpenFile( IxStream*)
#endif
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile IxStream");
#ifdef FEATURE_FILESOURCE_DRM_DCF
  if((m_bEveryThingOK) && (FS_STATE_INIT == m_eState) )
  {
    m_pIxStream = pInputStream;
    if (m_bFSAsync)
    {
      /* Post the event to the file open handler */
      MM_Signal_Set( m_pOpenFileSignal );
    }
    else
    {
      HandleOpenFileCommand();
    }
    status = FILE_SOURCE_SUCCESS;
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::OpenFile FS is not in valid state %d", m_eState);
  }
#endif
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "FileSource::OpenFile IxStream* status %d",status);
  return status;
}

/*! ======================================================================
   @brief   Close the existing opened file

   @detail     Please see the note below

   @param[in]  None
   @param[out] None
   @return     filesource status.

   @note       Once close is called, FileSource object will no longer support any
               API except OpenFile.
========================================================================== */
FileSourceStatus FileSourceHelper::CloseFile()
{
  FileSourceStatus status = FILE_SOURCE_SUCCESS;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::CloseFile");

  //Notify each media to abort the playback as close is being called
  if(m_pAudioFileHandle)
  {
    m_pAudioFileHandle->setMediaAbort();
  }
  if(m_pVideoFileHandle)
  {
    m_pVideoFileHandle->setMediaAbort();
  }
  if(m_pTextFileHandle)
  {
    m_pTextFileHandle->setMediaAbort();
  }

  m_bClosePending = true;
  //Make sure thread was started successfully before posting close command
  if(m_bEveryThingOK && m_bFSAsync)
  {
    /* Post the event to FILESOURCE thread to process close */
    MM_Signal_Set( m_pCloseFileSignal );
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::CloseFile waiting for close to complete");

    uint32 *pEvent = NULL;
    bool bRun = true;

    int bTimedOut = 0;
    //wait for CLOSE_FILE_DONE_EVENT signal from FILESOURCE thread
    while(bRun)
    {
      bTimedOut=0;
      if ( 0 == MM_SignalQ_TimedWait( m_pCloseDoneSignalQ, 2000, (void **) &pEvent, &bTimedOut ) )
      {
        if(!bTimedOut)
        {
          //No time out, check the event
          switch ( *pEvent )
          {
            case CLOSE_FILE_DONE_EVENT:
            {
              bRun = false;
              status = FILE_SOURCE_SUCCESS;
              break;
            }
          }
        }//if(!bTimedOut)
      }//if( 0 == MM_SignalQ_TimedWait
      else
      {
        //Exit otherwise we are stuck in infinite loop
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                       "FileSource::CloseFile MM_SignalQ_TimedWait returned FAILURE!!");
        bRun = false;
        status = FILE_SOURCE_FAIL;
      }
    }//while(bRun)
  }//if(m_bEveryThingOK)
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "FileSource::SourceThread received CLOSE_FILE_EVENT");
    HandleCloseFileCommand();
  }

  if(m_pCriticalSection != NULL)
  {
    MM_CriticalSection_Release(m_pCriticalSection);
    m_pCriticalSection = NULL;
  }
  if(m_pAudioCriticalSection != NULL)
  {
    MM_CriticalSection_Release(m_pAudioCriticalSection);
    m_pAudioCriticalSection = NULL;
  }
  if(m_pVideoCriticalSection != NULL)
  {
    MM_CriticalSection_Release(m_pVideoCriticalSection);
    m_pVideoCriticalSection = NULL;
  }
  if(m_pTextCriticalSection != NULL)
  {
    MM_CriticalSection_Release(m_pTextCriticalSection);
    m_pTextCriticalSection = NULL;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::CloseFile Close is complete");
  return status;
}
/*! ======================================================================
  @brief    method returns the file format.

  @param[out]  fileFormat   type of  FileFormat ( MPEG4/ASF/AVI).
  @return      filesource status.

  @note    It is expected that OpenFile() is called before getFileFormat() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::GetFileFormat(FileSourceFileFormat& fileFormat)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  fileFormat = FILE_SOURCE_UNKNOWN;
  FileBase* fileHandle = NULL;

  //select a file handle
  if (m_pAudioFileHandle)
  {
    fileHandle = m_pAudioFileHandle;
  }
  else if (m_pVideoFileHandle)
  {
    fileHandle = m_pVideoFileHandle;
  }
  else if (m_pTextFileHandle)
  {
    fileHandle = m_pTextFileHandle;
  }
  if (fileHandle)
  {
    status = fileHandle->GetFileFormat(fileFormat);
  }
  return status;
}

/*! ======================================================================
  @brief    API returns the DRM scheme used, if any
  @param[out]  drmtype   type of  DRM. Please refer to FileSourceDrmType enum.
  @return      filesource status.
  @note    It is expected that user has received the successul callback for OpenFile() earlier.
======================================================================  */
FileSourceStatus FileSourceHelper::GetDRMType(FileSourceDrmType& drmtype)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  drmtype = FILE_SOURCE_NO_DRM;
  status = FILE_SOURCE_SUCCESS;
  if (m_pAudioFileHandle && m_pAudioFileHandle->IsDRMProtection())
  {
    drmtype = FILE_SOURCE_DRM_UNKNOWN;
    status = m_pAudioFileHandle->GetDRMType(drmtype);
  }
  else if (m_pVideoFileHandle && m_pVideoFileHandle->IsDRMProtection())
  {
    drmtype = FILE_SOURCE_DRM_UNKNOWN;
    status = m_pVideoFileHandle->GetDRMType(drmtype);
  }
  else if (m_pTextFileHandle && m_pTextFileHandle->IsDRMProtection())
  {
    drmtype = FILE_SOURCE_DRM_UNKNOWN;
    status = m_pTextFileHandle->GetDRMType(drmtype);
  }
  return status;
}

/*! ======================================================================
  @brief    API to retrieve JANUS DRM information/header.

  @param[in/out]  nsize   Size of the drm header/information.
  @param[out]     pdata   points to memory to be filled in by filesource
  @return         filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
           Call the API with NULL pdata to get the size needed for storing drm header/info.
           User needs to allocate the memory and call again with valid memory address.
======================================================================  */
FileSourceStatus FileSourceHelper::GetJanusDRMInfo(void* pdata,uint32* nsize)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  if (m_pAudioFileHandle && m_pAudioFileHandle->IsDRMProtection())
  {
    status = m_pAudioFileHandle->GetJanusDRMInfo(pdata,nsize);
  }
  else if (m_pVideoFileHandle && m_pVideoFileHandle->IsDRMProtection())
  {
    status = m_pVideoFileHandle->GetJanusDRMInfo(pdata,nsize);
  }
  else if (m_pTextFileHandle && m_pTextFileHandle->IsDRMProtection())
  {
    status = m_pTextFileHandle->GetJanusDRMInfo(pdata,nsize);
  }
  return status;
}

/*! ======================================================================
  @brief    method returns Major and Minor Type for the Clip.

  @param[in]   id          track ID
  @param[out]  majorType   Major Media Types ( Audio/Video/Text).
  @param[out]  minorType   Minor Media Types ( Sub media type within Audio/Video text)
                           They are also referred as Codecs
  @return      filesource status.

  @note    It is expected that OpenFile() is called before getMimeType() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::GetMimeType (uint32 id,
                                                FileSourceMjMediaType& majorType,
                                                FileSourceMnMediaType& minorType
                                               )
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  // retrieve the major media type
  majorType = GetFileSourceMajorType(id);

  // retrieve the minor media type
  minorType = GetFileSourceMinorType(id);
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::getMimeType");

  if ( (majorType != FILE_SOURCE_MJ_TYPE_UNKNOWN ) && \
       (minorType != FILE_SOURCE_MN_TYPE_UNKNOWN)
     )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                "FileSource::getMimeType located major and minor type");
    status = FILE_SOURCE_SUCCESS;
  }
  return status;
}

/*! ======================================================================
   @brief    Provides Media Samples for requested tracks

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] pSampleInfo Provides Information( eg: timestamp etc) about the sample
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextMediaSample() is called.
========================================================================== */

FileSourceMediaStatus FileSourceHelper::GetNextMediaSample(uint32 id, uint8 *buf,
                                                           uint32 *size,
                                                           FileSourceSampleInfo& pSampleInfo)
{
  FileSourceMediaStatus status = FILE_SOURCE_DATA_ERROR;
  FileSourceMediaStatus eSampleInfoStatus = FILE_SOURCE_DATA_ERROR;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  FileSourceMjMediaType majorType;
  int  numOfSamples =1;
  int *numSample = &numOfSamples;
  majorType = GetFileSourceMajorType(id);
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
    "GetNextMediaSample request for trackID %lu, type %d",
    id, majorType);

  switch (majorType)
  {
  case FILE_SOURCE_MJ_TYPE_AUDIO:
    // get the next Audio sample
    if(m_pAudioCriticalSection)
    {
      MM_CriticalSection_Enter(m_pAudioCriticalSection);
    }
    /* To support Gapless playback feature, Parser will give EOF as part of
       last valid media sample itself. So treat DATA End also as valid error
       code in this API call. */
    status = GetNextAudioFrame(id, buf, size, numSample);
    if((FILE_SOURCE_DATA_OK == status) ||
       (FILE_SOURCE_DATA_END == status))
    {
      eSampleInfoStatus = GetCurrentSampleInfo(id, pSampleInfo,
                                               m_pAudioFileHandle );
      if (FILE_SOURCE_DATA_END != status)
      {
        status = eSampleInfoStatus;
      }
    }
    if(m_pAudioCriticalSection)
    {
      MM_CriticalSection_Leave(m_pAudioCriticalSection);
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "AUDIO:GetNextMediaSample TS %llu, size %lu",
                 pSampleInfo.startTime, *size);
    break;

  case FILE_SOURCE_MJ_TYPE_VIDEO:
    // get the next Video sample
    if(m_pVideoCriticalSection)
    {
      MM_CriticalSection_Enter(m_pVideoCriticalSection);
    }
    status = GetNextVideoSample(id, buf, size, numSample);
    if((FILE_SOURCE_DATA_OK == status) ||
       (FILE_SOURCE_DATA_END == status))
    {
      eSampleInfoStatus = GetCurrentSampleInfo(id, pSampleInfo,
                                               m_pVideoFileHandle );
      if (FILE_SOURCE_DATA_END != status)
      {
        status = eSampleInfoStatus;
      }
    }
    if(m_pVideoCriticalSection)
    {
      MM_CriticalSection_Leave(m_pVideoCriticalSection);
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "VIDEO:GetNextMediaSample TS %llu, size %lu",
                 pSampleInfo.startTime, *size);
    break;

  case FILE_SOURCE_MJ_TYPE_TEXT:
    // get the next Text sample
    if(m_pTextCriticalSection)
    {
      MM_CriticalSection_Enter(m_pTextCriticalSection);
    }
    status = GetNextTextSample(id, buf, size, numSample);
    if((FILE_SOURCE_DATA_OK == status) ||
       (FILE_SOURCE_DATA_END == status))
    {
      eSampleInfoStatus = GetCurrentSampleInfo(id, pSampleInfo,
                                               m_pTextFileHandle );
      if (FILE_SOURCE_DATA_END != status)
      {
        status = eSampleInfoStatus;
      }
    }
    if(m_pTextCriticalSection)
    {
      MM_CriticalSection_Leave(m_pTextCriticalSection);
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "TEXT:GetNextMediaSample TS %llu, size %lu",
                 pSampleInfo.startTime, *size);

    break;
  default:
    ;/* Do nothing */
  }
  //print if return status is some error
  if(status != FILE_SOURCE_DATA_OK)
  {
     MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "GetNextMediaSample for id %lu return status %d",
               id, status);
  }
  return status;
}

/*! ======================================================================================================
   @brief    Provides the buffered duration of a track at any point of time during progressive download

   @param[in]  id     The track ID.
   @param[in]  nBytes Buffered/downloaded bytes
   @param[out] pDuration Function will copy buffered duration into this variable
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getBufferedDuration() is called.
=========================================================================================================*/
FileSourceStatus FileSourceHelper::GetBufferedDuration(uint32 ulTrackID,
                                                       int64 nBytes,
                                                       uint64 *pDuration)
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return eStatus;
  }
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  /************************************************************************/
  /* Check File format type for given Track ID. If File format is audio   */
  /* only container format, then get file size and duration from the      */
  /* corresponding parser and calculate the approximate duration value for*/
  /* the downloaded offset value.                                         */
  /* For A-V containers, corresponding parsers will have implementations. */
  /************************************************************************/
  FileBase *pFileBaseHandle       = GetMediaHandleForTrackID(ulTrackID);
  void     *pCriticalSection      = GetCriticalSectionPtrForTrackID(ulTrackID);
  FileSourceFileFormat eFileFormat= FILE_SOURCE_UNKNOWN;
  if(pFileBaseHandle)
  {
    pFileBaseHandle->GetFileFormat(eFileFormat);
  }
  switch(eFileFormat)
  {
    case FILE_SOURCE_AAC:
    case FILE_SOURCE_AC3:
    case FILE_SOURCE_AMR_NB:
    case FILE_SOURCE_AMR_WB:
    case FILE_SOURCE_DTS:
    case FILE_SOURCE_FLV:
    case FILE_SOURCE_MP3:
    case FILE_SOURCE_OGG:
    case FILE_SOURCE_QCP:
    case FILE_SOURCE_WAV:
    {
      uint64 ullFileSize = pFileBaseHandle->GetFileSize();
      uint64 ullDuration = pFileBaseHandle->getMovieDuration();
      uint32 ulTimeScale = pFileBaseHandle->getMovieTimescale();
      if (-1 == nBytes)
      {
        int64 nDownloadedBytes = 0;
        bool  bEndOfData       = false;
        m_pIStreamPort->GetAvailableOffset(&nDownloadedBytes, &bEndOfData);
        nBytes = nDownloadedBytes;
      }
      if ((ullFileSize) && (MAX_FILE_SIZE != ullFileSize) && (ullDuration) &&
          (-1 != nBytes))
      {
        *pDuration = (nBytes * ullDuration) / ullFileSize;
        //! Convert timestamp in milli-sec units. User expects output of this
        //! call in milli-sec only.
        *pDuration = (*pDuration * MILLISEC_TIMESCALE_UNIT) / ulTimeScale;
        eStatus    = FILE_SOURCE_SUCCESS;
      }
    }
    break;
    default:
    {
      eStatus = FILE_SOURCE_FAIL;
    }
    break;
  }
  if ((FILE_SOURCE_SUCCESS != eStatus) && (pFileBaseHandle) &&
      (pCriticalSection))
  {
    MM_CriticalSection_Enter(pCriticalSection);
    eStatus = pFileBaseHandle->getBufferedDuration(ulTrackID, nBytes, pDuration)? \
              FILE_SOURCE_SUCCESS:FILE_SOURCE_FAIL;
    MM_CriticalSection_Leave(pCriticalSection);
  }
#else
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
       "FileSource::FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD not enabled");
  eStatus = FILE_SOURCE_NOTAVAILABLE;
#endif
  return eStatus;
}

/*! ======================================================================
   @brief    Proves Current Position in micro seconds

   @detail    This function retrieves the current position without you having to call
      the getNextMediaSample().

   @param[in]  id    The track ID of the track
   @return     returns the timestamp in microseconds of the next valid sample.

   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
========================================================================== */

uint64 FileSourceHelper::GetMediaCurrentPosition(uint32 id)
{
  uint64 StartTime = 0;
  uint64 mediaTS = 0;
  uint32 mediaTimeScale = 0;
  FileSourceMjMediaType majorType;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return StartTime;
  }
  majorType = GetFileSourceMajorType(id);

  switch (majorType)
  {
  case FILE_SOURCE_MJ_TYPE_AUDIO:
    if(m_pAudioFileHandle)
    {
      mediaTS = m_pAudioFileHandle->getMediaTimestampForCurrentSample(id);
      mediaTimeScale = m_pAudioFileHandle->getTrackMediaTimescale(id);
    }
    break;

  case FILE_SOURCE_MJ_TYPE_VIDEO:
    if(m_pVideoFileHandle)
    {
      mediaTS = m_pVideoFileHandle->getMediaTimestampForCurrentSample(id);
      mediaTimeScale = m_pVideoFileHandle->getTrackMediaTimescale(id);
    }
    break;

  case FILE_SOURCE_MJ_TYPE_TEXT:
    if(m_pTextFileHandle)
    {
      mediaTS = m_pTextFileHandle->getMediaTimestampForCurrentSample(id);
      mediaTimeScale = m_pTextFileHandle->getTrackMediaTimescale(id);
    }
    break;
  default:
    ;/* Do nothing */
  }

  if (mediaTimeScale > 0)
  {
    StartTime = (MICROSEC_TIMESCALE_UNIT * mediaTS)/ mediaTimeScale;
  }
  return StartTime;
}

/*! ======================================================================
  @brief    Proves Current Position in micro seconds for the entire track

   @detail    This function retrieves the current position without you having to call
             the getNextMediaSample(). This function gives preference to video track
       position over audio and audio over text to calculate the current position

   @return     returns the timestamp in microseconds of the next valid sample.

   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
  ========================================================================== */

uint64 FileSourceHelper::GetMediaCurrentPosition()
{
  uint64 time = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return time;
  }

  if ((m_pVideoFileHandle)&& (m_videoSelectedTrackId >=0))
  {
    //retrieve the current position of the video track
    time = GetMediaCurrentPosition(m_videoSelectedTrackId);
  }
  else if ((m_pAudioFileHandle)&& (m_audioSelectedTrackId >=0))
  {
    // retrieve the current position of the audio track
    time = GetMediaCurrentPosition(m_audioSelectedTrackId);
  }
  else if ((m_pTextFileHandle)&& (m_textSelectedTrackId >=0))
  {
    // retrieve the current position of the audio track
    time = GetMediaCurrentPosition(m_textSelectedTrackId);
  }
  return time;
}

/*! ======================================================================
  @brief      Method to retrieve configuration item set previously via SetConfiguration.

  @param[in]      id            Track id to identify track to which configuration data belongs.
  @param[out]     pItem         Configuration data filled in by parser
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */

FileSourceStatus FileSourceHelper::GetConfiguration(uint32 id,
                                                  FileSourceConfigItem* pItem,
                                                  FileSourceConfigItemEnum ienumData)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  FileBase* clipFileBase = NULL;
  FileSourceMjMediaType majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
  FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
  //Get media mime-type
  status = GetMimeType(id,majorType,minorType);
  //Get media handle
  clipFileBase = GetMediaHandleForTrackID(id);
  // GetConfiguration() API can be serve only if FileSource is in valid state
  // i.e. FILESOURCE_READY.
  if ( ( pItem != NULL ) &&
       ( clipFileBase != NULL ) &&
       ( IS_FILESOURCE_STATUS_OK(m_eState) == true ) )
  {
    memset(pItem,0,sizeof(FileSourceConfigItem));
    switch(ienumData)
    {
    case FILE_SOURCE_MEDIA_BASETIME:
      {
        uint64* baseTime = &pItem->nresult;
        if( true == clipFileBase->getBaseTime( id,baseTime ))
        {
          uint32 ulScale = clipFileBase->getTrackMediaTimescale(id);
          if(MICROSEC_TIMESCALE_UNIT != ulScale)
            *baseTime = ((*baseTime) * MICROSEC_TIMESCALE_UNIT)/ulScale;
          status = FILE_SOURCE_SUCCESS;
        }
        else
        {
          *baseTime = (uint64)-1; //Set with largest possible value
        }
      }
      break;//case getBaseTime()

    case FILE_SOURCE_MEDIA_PROFILE_INFORMATION:
      {
        FS_VIDEO_PARAM_H264TYPE sH264Info;
        memset(&sH264Info, 0, sizeof(FS_VIDEO_PARAM_H264TYPE));
        if( ( FILE_SOURCE_MJ_TYPE_VIDEO == majorType ) &&
            ( FILE_SOURCE_MN_TYPE_H264 == minorType ) &&
            ( PARSER_ErrorNone ==
                  clipFileBase->GetStreamParameter(id,
                                                   FS_IndexParamVideoH264,
                                                   &sH264Info ) ) )
        {
          pItem->nresult = sH264Info.ucH264ProfileInfo;
          status = FILE_SOURCE_SUCCESS;
        }
      }
      break;//case Profile information

    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
      {
        bool bStatus = false;
        status = clipFileBase->GetAudioOutputMode(&bStatus,
                                                  ienumData);
        pItem->nresult = bStatus;
      }
      break; //case GetAudioOutputMode()

    case FILE_SOURCE_MEDIA_BIT_STREAM_INFORMATION:
      {
        if(FILE_SOURCE_MJ_TYPE_AUDIO == majorType)
        {
          switch(minorType)
          {
            case FILE_SOURCE_MN_TYPE_AC3:
            case FILE_SOURCE_MN_TYPE_EAC3:
            case FILE_SOURCE_MN_TYPE_EAC3_JOC:
              {
                FS_AUDIO_PARAM_AC3TYPE*  pAC3BSI =
                                  (FS_AUDIO_PARAM_AC3TYPE*)pItem->nresult;
                if(pAC3BSI)
                {
                  memset(pAC3BSI, 0, sizeof(FS_AUDIO_PARAM_AC3TYPE));
                }
                if( pAC3BSI && PARSER_ErrorNone ==
                    clipFileBase->GetStreamParameter(id,
                                                     FS_IndexParamAudioAC3,
                                                     pAC3BSI ) )
                {
                  status = FILE_SOURCE_SUCCESS;
                }
              }
              break;
            case FILE_SOURCE_MN_TYPE_DTS:
              {
                FS_AUDIO_PARAM_DTSTYPE*  pDTSBSI =
                                  (FS_AUDIO_PARAM_DTSTYPE*)pItem->nresult;
                if(pDTSBSI)
                {
                  memset(pDTSBSI, 0, sizeof(FS_AUDIO_PARAM_DTSTYPE));
                }
                if( pDTSBSI && PARSER_ErrorNone ==
                    clipFileBase->GetStreamParameter(id,
                                                     FS_IndexParamAudioDTS,
                                                     pDTSBSI ) )
                {
                  status = FILE_SOURCE_SUCCESS;
                }
              }
              break;
            default:
              {
                //Invalid parameter passed through this API
                status = FILE_SOURCE_INVALID;
              }
              break;
          }
        }
      }
      break; //case Get BSI Information

    case FILE_SOURCE_MEDIA_NUM_DRM_SYSTEM_SUPPORTED:
      {
        uint32 nDRMSystems = 0;
        status = clipFileBase->GetNumberOfDRMSupported(&nDRMSystems);
        pItem->nresult = nDRMSystems;
      }
      break; //case Number of DRM Supported

    default:
      {
        //Invalid parameter passed through this API
        status = FILE_SOURCE_INVALID;
      }
      break;
    }//switch(ienumData)
  }// if(pItem && clipFileBase)
  else
  {
    //Invalid parameter passed through this API
    status = FILE_SOURCE_INVALID;
  }
  return status;
}
/*! ======================================================================
  @brief      Method to Set configuration item.

  @param[in]      id            Track id to identify track to which configuration data belongs.
  @param[out]     pItem         Configuration data filled in by caller
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */

FileSourceStatus FileSourceHelper::SetConfiguration(uint32 id, FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;

  FileBase* clipFileBase = NULL;
  status = FILE_SOURCE_DATA_NOTAVAILABLE;
  bool nRet = false;
  clipFileBase = GetMediaHandleForTrackID(id);
  if( (ienumData == FILE_SOURCE_MEDIA_BASETIME)&& pItem)
  {
    if (clipFileBase && pItem)
    {
      uint32 ulScale = clipFileBase->getTrackMediaTimescale(id);
      uint64 ullTime = ((uint64)pItem->nresult * ulScale) /
                       MICROSEC_TIMESCALE_UNIT;
      nRet = clipFileBase->setBaseTime(id, ullTime);
      if(nRet)
      {
        status = FILE_SOURCE_SUCCESS;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "FileSource::SetClipMetaData clipFileBase or pMetaData is NULL");
    }
  }
  else if( (ienumData == FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION) ||
           (ienumData == FILE_SOURCE_MEDIA_DISABLE_TS_DISCONTINUITY_CORRECTION) )
  {
    if(pItem && clipFileBase )
    {
      pItem->nresult = 0;
      status = clipFileBase->SetConfiguration(ienumData);
      if(status == FILE_SOURCE_SUCCESS)
      {
        pItem->nresult =1;
      }
    }
  }
  else if( (ienumData == FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE) ||
           (ienumData == FILE_SOURCE_MEDIA_DISABLE_PCM_SAMPLE_UPGRADE) )
  {
    if(pItem && clipFileBase )
    {
      pItem->nresult = 0;
      status = clipFileBase->SetConfiguration(ienumData);
      if(status == FILE_SOURCE_SUCCESS)
      {
        pItem->nresult = 1;
      }
    }
  }
  else if(pItem && clipFileBase )
  {
    FileSourceFileFormat fileFormat = FILE_SOURCE_UNKNOWN;
    pItem->nresult = 0;
    status = clipFileBase->SetAudioOutputMode(ienumData);
    if(status == FILE_SOURCE_SUCCESS)
    {
      pItem->nresult =1;
    }
    if((ienumData == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME) &&
       (GetFileFormat(fileFormat) == FILE_SOURCE_SUCCESS) &&
       (m_pAudioFileHandle))
    {
      uint8 ucAudioType = m_pAudioFileHandle->getTrackOTIType(id);
      //Also check audio format here
      if  (((fileFormat==FILE_SOURCE_AVI)&&((ucAudioType==MP3_AUDIO) ||
            (ucAudioType==AAC_ADTS_AUDIO) || (ucAudioType==AC3_AUDIO))) ||
          ((fileFormat==FILE_SOURCE_MP2TS)&&(ucAudioType==AC3_AUDIO)) ||
          ((fileFormat==FILE_SOURCE_WFD_MP2TS)&&(ucAudioType==AC3_AUDIO)) ||
          ((fileFormat==FILE_SOURCE_DASH_MP2TS)&&(ucAudioType==AC3_AUDIO)))
      {
        if(!m_pAudioDataBuffer)
        {
          m_pAudioDataBuffer = (audio_data_buffer*)MM_Malloc(sizeof(audio_data_buffer));
          if(m_pAudioDataBuffer)
          {
            memset(m_pAudioDataBuffer,0,sizeof(audio_data_buffer));
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
              "FileSourceHelper::GetNextAudioFrame MM_Malloc failed");
            return FILE_SOURCE_FAIL;
          }
        }
      }
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "FileSource::SetConfiguration failed.. %d",ienumData);
  }
  return status;
}

/*! ======================================================================
    @brief      method returns metadata about the Clip.

   @details     In case of MPEG4 file format it retrieves the information from the
               PVUserDataAtom 'pvmm' atom.

   @param[in]  ienumData      Identifies requested metadata type( eg: Title, Author etc).
   @param[out] oMetaData      returns a string associated with requested metadata
   @return     file source status

   @note    It is expected that OpenFile() is called before getClipMetaData() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::GetClipMetaData(wchar_t* pMetaData,
                                             uint32* pLength,
                                             FileSourceMetaDataType ienumData,
                                             FS_TEXT_ENCODING_TYPE* peEncodeType)
{
  FileSourceStatus status = FILE_SOURCE_INVALID;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  FileBase* clipFileBase = NULL;
  bool fileStatus = false;

  if (m_pVideoFileHandle)
  {
    clipFileBase = m_pVideoFileHandle;
  }
  else if (m_pAudioFileHandle)
  {
    clipFileBase = m_pAudioFileHandle;
  }
  else if (m_pTextFileHandle)
  {
    clipFileBase = m_pTextFileHandle;
  }

  if(pLength && clipFileBase)
  {
    PARSER_ERRORTYPE retStatus = PARSER_ErrorNotImplemented;
    switch (ienumData)
    {
      case FILE_SOURCE_MD_ALBUM_ART:
        retStatus = clipFileBase->getAlbumArt(pMetaData, pLength);
      break;
      default:
        retStatus = clipFileBase->GetClipMetaData(pMetaData, pLength, ienumData);
        //! Check whether data in output buffer is in UTF8 format or not.
        if (peEncodeType && pMetaData)
        {
          //! Initialize the variable
          *peEncodeType = FS_ENCODING_TYPE_UNKNOWN;
          *peEncodeType = clipFileBase->GetMetaDataEncodingType();
          //! By default all Unknown Encodings will be mapped to UTF32
          //! Some Parsers are doing this conversion,
          //! This conversion will be removed from each parser on need basis
          if (FS_ENCODING_TYPE_UNKNOWN == *peEncodeType)
          {
            *peEncodeType = FS_TEXT_ENC_UTF32;
          }
        }
      break;
    }
    if(PARSER_ErrorNotImplemented != retStatus)
    {
      if(PARSER_ErrorNone == retStatus)
      {
        return FILE_SOURCE_SUCCESS;
      }
      else
      {
        return FILE_SOURCE_FAIL;
      }
    }
  }

  if (clipFileBase && pLength)
  {
    FILESOURCE_STRING metaData;
    status = FILE_SOURCE_DATA_NOTAVAILABLE;
    switch (ienumData)
    {
      case FILE_SOURCE_MD_TITLE:
      metaData = clipFileBase->getTitle();
      break;
      case FILE_SOURCE_MD_AUTHOR:
      metaData = clipFileBase->getAuthor();
      break;
      case FILE_SOURCE_MD_DESCRIPTION:
      metaData = clipFileBase->getDescription();
      break;
      case FILE_SOURCE_MD_RATING:
      metaData = clipFileBase->getRating();
      break;
      case FILE_SOURCE_MD_COPYRIGHT:
      metaData = clipFileBase->getCopyright();
      break;
      case FILE_SOURCE_MD_VERSION:
      metaData = clipFileBase->getVersion();
      break;
      case FILE_SOURCE_MD_CREATION_DATE:
      metaData = clipFileBase->getCreationDate();
      break;
      case FILE_SOURCE_MD_PERFORMANCE:
      metaData = clipFileBase->getPerf();
      break;
      case FILE_SOURCE_MD_GENRE:
      metaData = clipFileBase->getGenre();
      break;
      case FILE_SOURCE_MD_CLASSIFICATION:
      metaData = clipFileBase->getClsf();
      break;
      case FILE_SOURCE_MD_KEYWORD:
      metaData = clipFileBase->getKywd();
      break;
      case FILE_SOURCE_MD_LOCATION:
      metaData = clipFileBase->getLoci();
      break;
      case FILE_SOURCE_MD_INFO:
      case FILE_SOURCE_MD_OWNER:
      default:
      metaData = NULL;
    }
    fileStatus = clipFileBase->FileSuccess();
    if(
        (clipFileBase)              &&
        (fileStatus == true)        &&
        (metaData.get_cstr() != NULL)  )
    {
      if( (pMetaData) && (*pLength  >= (uint32)metaData.get_size()))
      {
#ifdef PLATFORM_LTK
       wcscpy_s(pMetaData,*pLength,metaData.get_cstr());
#else
       zrex_wcscpy(pMetaData,*pLength,metaData.get_cstr());
#endif
      }
      else
      {
        *pLength = metaData.get_size()+1;
      }
      status = FILE_SOURCE_SUCCESS;
    }
  }
  return status;
}
/*! ======================================================================
   @brief      method to retrieve all valid trackID List

   @param[out]  trackIdInfo   a list of TrackIfInfo (
                consisting of trackid's and if a book if they are selected or not).
   @return     number of valid audio, video and text tracks.

   @note    It is expected that OpenFile() is called before getWholeTracksIDList() is called.
========================================================================== */

uint32 FileSourceHelper::GetWholeTracksIDList(FileSourceTrackIdInfoType *trackIdInfo )
{
  uint32 totalNumTracks = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return totalNumTracks;
  }
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
       "FileSource::getWholeTracksIDList #video %lu #audio %lu #text %lu ",
       m_nVideoTracks, m_nAudioTracks, m_nTextTracks);
  totalNumTracks = m_nVideoTracks + m_nAudioTracks + m_nTextTracks;

  // deselect non relevent tracks
  for ( uint32 i = totalNumTracks ; i < FILE_SOURCE_MAX_NUM_TRACKS; i++)
  {
    m_trackIdInfo[i].id = 0;
    m_trackIdInfo[i].selected = false;
  }

  if ( trackIdInfo != NULL)
  {
    // select the relevant tracks and copy the information
    for (uint32 i=0; i< totalNumTracks; i++)
    {
      trackIdInfo[i].id = m_trackIdInfo[i].id;
      trackIdInfo[i].selected = m_trackIdInfo[i].selected;
      trackIdInfo[i].majorType = m_trackIdInfo[i].majorType;
    }
  }

  return totalNumTracks;
}

/*! ======================================================================
   @brief      method for User to Select a Track.

   @param[in]  id   track Id to select
   @return     file source status.

   @note    It is expected that OpenFile() is called before setSelectedTrackID() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::SetSelectedTrackID(uint32 id)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  FileSourceMjMediaType majorType;
  uint32 i = 0;
  int index;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FileSource::setSelectedTrackID id %lu",id);

  if ( ((int)id == m_audioSelectedTrackId) || \
       ((int)id == m_videoSelectedTrackId) || \
       ((int)id == m_textSelectedTrackId) \
     )
  {
    status = FILE_SOURCE_SUCCESS;
  }
  else
  {

    majorType = GetFileSourceMajorType(id);

    //un select the non-relevant tracks
    switch (majorType)
    {
    case FILE_SOURCE_MJ_TYPE_AUDIO:
      for (i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
      {
        if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
        {
          m_trackIdInfo[i].selected = false;
        }
      }
      m_audioSelectedTrackId = (int)id;
      break;

    case FILE_SOURCE_MJ_TYPE_VIDEO:
      for (i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
      {
        if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
        {
          m_trackIdInfo[i].selected = false;
        }
      }
      m_videoSelectedTrackId = (int)id;
      break;

    case FILE_SOURCE_MJ_TYPE_TEXT:
      for (i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
      {
        if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_TEXT)
        {
          m_trackIdInfo[i].selected = false;
        }
      }
      m_textSelectedTrackId = (int)id;
      break;
    default:
      ;/* Do nothing */
    }

    // select the relevant tracks
    if ((index = GetIndexInTrackIdInfo(id)) >= 0)
    {
      m_trackIdInfo[index].selected = true;
      status = FILE_SOURCE_SUCCESS;
    }

  }
  return status;
}

/*! ======================================================================
  @brief      Provides information about the Track.

   @detail    method to retrieve information about the Audio/Video Track. This Interface is provided
              for User to do channel selection OR use it as a criteria to select a particular track.

   @param[in]  id   track Id to select
   @param[out] info Information about the given track id
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getMediaTrackInfo() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::GetMediaTrackInfo(uint32 id,MediaTrackInfo* info)
{
  FileSourceMjMediaType majorType;
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::getMediaTrackInfo id =%lu",id);
  if(info)
  {
    memset(info,0,sizeof(MediaTrackInfo));
    int index = GetIndexInMediaTrackInfo(id);

    if (index >= 0)
    {
      majorType = GetFileSourceMajorType(id);
      switch (majorType)
      {
        case FILE_SOURCE_MJ_TYPE_AUDIO:
        {
          memcpy(info, &m_audioTrackInfo[index], sizeof(MediaTrackInfo));
          status = FILE_SOURCE_SUCCESS;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::getMediaTrackInfo FILE_SOURCE_MJ_TYPE_AUDIO");
        }
        break;

        case FILE_SOURCE_MJ_TYPE_VIDEO:
        {
          memcpy(info, &m_videoTrackInfo[index], sizeof(MediaTrackInfo));
          status = FILE_SOURCE_SUCCESS;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::getMediaTrackInfo FILE_SOURCE_MJ_TYPE_VIDEO");
        }
        break;

        case FILE_SOURCE_MJ_TYPE_TEXT:
        {
          memcpy(info, &m_textTrackInfo[index], sizeof(MediaTrackInfo));
          status = FILE_SOURCE_SUCCESS;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::getMediaTrackInfo FILE_SOURCE_MJ_TYPE_TEXT");
        }
        break;
        default:
        ;/* Do nothing */
      }
    }
  }
  return status;
}
/*! ======================================================================
   @brief      Provides information about the Track.

   @details    Method to set information about the Audio/Video Track.

   @param[in] info Information about the codec/track
   @return     FILE_SOURCE_SUCCESS if successful in setting the information otherwise returns appropriate error.

   @note       It is expected that user has called OpenFile with FILE_SOURCE_RAW and has
               received OPEN_COMPLETE before invoking this API.

========================================================================== */
FileSourceStatus FileSourceHelper::SetMediaTrackInfo(MediaTrackInfo info)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  status = FILE_SOURCE_INVALID;

  if( (m_hFileFormatRequested == FILE_SOURCE_RAW ) &&
    (info.audioTrackInfo.audioCodec != FILE_SOURCE_MN_TYPE_UNKNOWN))
  {
    if(m_pAudioFileHandle && m_pAudioFileHandle->setAudioInfo(info.audioTrackInfo))
    {
      status = FILE_SOURCE_SUCCESS;
      //Update track/codec information
      FillAudioTrackInfo();
      SelectDefaultTracks();
    }
  }
  if( (m_hFileFormatRequested == FILE_SOURCE_RAW_H265 ) &&
    (info.videoTrackInfo.videoCodec != FILE_SOURCE_MN_TYPE_UNKNOWN))
  {
    if( m_pVideoFileHandle && m_pVideoFileHandle->setVideoInfo(info.videoTrackInfo) )
    {
      status = FILE_SOURCE_SUCCESS;
      //Update track/codec information
      FillVideoTrackInfo();
      SelectDefaultTracks();
    }
  }

  return status;
}

/*! ======================================================================
   @brief      Maximum Buffer size required for the Track.

   @detail    Before we parse a clip we do not know the size of the Frame.
              There are two ways to solve this issue. One is to allocate a huge memory buffer,
              which is in-efficient use of memory OR use this method to retrieve the
              buffer size needed for the frame and then allocate/reallocate memory as needed.

   @param[in]  id   track Id to select.

   @return  largest frame size up to the frame we have parsed.
      (Note:we do not parse/scan all the frames during initial parsing).

   @note    It is expected that OpenFile() is called before getTrackMaxFrameBufferSize() is called.
========================================================================== */

int32  FileSourceHelper::GetTrackMaxFrameBufferSize(uint32 id)
{
  int32 n = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return n;
  }
  FileSourceMjMediaType majorType;
  majorType = GetFileSourceMajorType(id);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FileSource::getTrackMaxFrameBufferSize id %lu",id);
  switch (majorType)
  {
  case FILE_SOURCE_MJ_TYPE_AUDIO:
    n = (int32)m_pAudioFileHandle->getTrackMaxBufferSizeDB(id);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FileSource::getTrackMaxFrameBufferSize AUDIO size %ld",n);
    break;

  case FILE_SOURCE_MJ_TYPE_VIDEO:
    n = (int32)m_pVideoFileHandle->getTrackMaxBufferSizeDB(id);
    /* Check whether Max Frame Buffer Size is less than Minimum Buf Size
       expected. Min Buf Size is SQCIF resolution equivalent value.
     */
    if(n < MIN_MEDIA_FRAME_SIZE)
    {
      n = (int32)MIN_MEDIA_FRAME_SIZE;
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FileSource::getTrackMaxFrameBufferSize VIDEO size %ld",n);
    break;

  case FILE_SOURCE_MJ_TYPE_TEXT:
    n = (int32)m_pTextFileHandle->getTrackMaxBufferSizeDB(id);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FileSource::getTrackMaxFrameBufferSize TEXT size %ld",n);
    break;
  default:
    ;/* Do nothing */
  }

  return n;
}

/*! ======================================================================
   @brief      gives the duration of the track in microseconds.

   @detail    This method retrives the duration from  the track.

   @param[in]  id   track Id to select.
   @return     duration of the track in micro seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
========================================================================== */

uint64  FileSourceHelper::GetTrackMediaDuration(uint32 id)
{
  uint64 trackDuration = 0;
  uint64 ullDur = 0;
  FileBase* trackFileBase = NULL;

  FileSourceMjMediaType majorType;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return trackDuration;
  }
  majorType = GetFileSourceMajorType(id);

  switch (majorType)
  {
  case FILE_SOURCE_MJ_TYPE_AUDIO:
    trackFileBase = m_pAudioFileHandle;
    break;

  case FILE_SOURCE_MJ_TYPE_VIDEO:
    trackFileBase = m_pVideoFileHandle;
    break;

  case FILE_SOURCE_MJ_TYPE_TEXT:
    trackFileBase = m_pTextFileHandle;
    break;
  default: ;
  }

  if (trackFileBase)
  {
    //Get the duration of the base audio track (index 0).
    ullDur = trackFileBase->getTrackMediaDuration(id);
    uint32 ulTimescale = trackFileBase->getTrackMediaTimescale(id);
    if ((ulTimescale) && (MICROSEC_TIMESCALE_UNIT != ulTimescale))
    {
      trackDuration = (MICROSEC_TIMESCALE_UNIT * ullDur) / ulTimescale;
    }
  }
  return trackDuration;
}

/*! ======================================================================
   @brief      gives the duration of the Clip in microseconds.

   @detail    This method retrives the duration from all the tracks ( audio, video and text).
              and picks the maximum.
   @return     duration of the clip in micro seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
========================================================================== */

uint64  FileSourceHelper::GetClipDuration()
{
  uint64 ullClipDur  = 0;
  uint64 ullVideoDur = 0;
  uint64 ullAudioDur = 0;
  uint64 ullTextDur  = 0;
  uint64 ullDuration = 0;
  uint32 ulTimeScale = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return ullClipDur;
  }

  // get duration of the selected media tracks
  if(m_pAudioFileHandle)
  {
    ullDuration = m_pAudioFileHandle->getMovieDuration();
    ulTimeScale = m_pAudioFileHandle->getMovieTimescale();
    if ((ulTimeScale) && (MICROSEC_TIMESCALE_UNIT != ulTimeScale))
    {
      ullAudioDur = (ullDuration * MICROSEC_TIMESCALE_UNIT) / ulTimeScale;
    }
  }

  if(m_pVideoFileHandle)
  {
    ullDuration = m_pVideoFileHandle->getMovieDuration();
    ulTimeScale = m_pVideoFileHandle->getMovieTimescale();
    if ((ulTimeScale) && (MICROSEC_TIMESCALE_UNIT != ulTimeScale))
    {
      ullVideoDur = (ullDuration * MICROSEC_TIMESCALE_UNIT) / ulTimeScale;
    }
  }

  if(m_pTextFileHandle)
  {
    ullDuration = m_pTextFileHandle->getMovieDuration();
    ulTimeScale = m_pTextFileHandle->getMovieTimescale();
    if ((ulTimeScale) && (MICROSEC_TIMESCALE_UNIT != ulTimeScale))
    {
      ullTextDur = (ullDuration * MICROSEC_TIMESCALE_UNIT) / ulTimeScale;
    }
  }

  /* get maximum of all the tracks as overall movie duration */
  ullClipDur = FILESOURCE_MAX(ullAudioDur, ullVideoDur);
  ullClipDur = FILESOURCE_MAX(ullClipDur, ullTextDur);

  /* return duration in ms */
  return ullClipDur;
}

/*! ==============================================================================================
   @brief      retrieve the Format Block information about the track.

   @detail    method to retrieve the Decoder specific/Format Block information from the track.
              This interface is generic for Audio, Video and Text.
              If buf = NULL, then the function give the size of the required buffer.
              Following is an example of retrieving the format block.

              1.Invoke getFormatBlock API for a given track identifier by passing in NULL for buf.
              2.If a track is valid, *pbufSize will give you the size of format block.
              3.Allocate the memory and invoke getFormatBlock API for a given track identifier
                by passing handle to allocated memory.

   @param[in]   id   track Id to select.
   @param[out]  buf   Buffer provies the format block info to the caller
   @param[out]  pBufSize   Size of the FormatBlock buffer
  @param[in]    bRawCodec  Flag to indicate whether codec data required in input format
                           or converted to proper SPS/PPS format

   @return     file source status.

   @note    It is expected that OpenFile() is called before getFormatBlock() is called.
  =================================================================================================*/

FileSourceStatus FileSourceHelper::GetFormatBlock(uint32 id,uint8* buf,
                                                  uint32 *pbufSize, bool bRawCodec)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }

  if(pbufSize)
  {
    FileSourceMjMediaType majorType;
    majorType = GetFileSourceMajorType(id);
    uint32 format_block_size = 0;
    FileBase* pBaseFile = NULL;

    switch (majorType)
    {
      case FILE_SOURCE_MJ_TYPE_AUDIO:
        pBaseFile = m_pAudioFileHandle;
      break;
      case FILE_SOURCE_MJ_TYPE_VIDEO:
        pBaseFile = m_pVideoFileHandle;
      break;
      case FILE_SOURCE_MJ_TYPE_TEXT:
         pBaseFile = m_pTextFileHandle;
      break;
      default:
        break;
    }
    if(pBaseFile)
    {
      pBaseFile->getCodecDatainRawFormat(bRawCodec);
      PARSER_ERRORTYPE errorCode =
        pBaseFile->getTrackDecoderSpecificInfoContent(id,NULL,&format_block_size);

      if(format_block_size)
      {
        if(buf && (*pbufSize >= format_block_size) )
        {
          errorCode = pBaseFile->getTrackDecoderSpecificInfoContent(id,buf,pbufSize);
          *pbufSize = format_block_size;
          status = FILE_SOURCE_SUCCESS;
        }
        else
        {
          *pbufSize = format_block_size;
          status = FILE_SOURCE_SUCCESS;
        }
      }
      else
      {
        //There is no FORMAT block.
        *pbufSize = 0;
        status = FILE_SOURCE_SUCCESS;
      }
    }//if(pBaseFile)
  }//if(pbufSize)
  return status;
}

/*! ======================================================================
   @brief      Reposition to an absolute position.

   @detail    The routine will seek/repositon to next valid sample based on the timestamp
              provided. This function Uses Video track as the primary source to find the time stamp and
              then sync's the Audio and Text accordingly. However, if Audio is not present it
              will use Audio as the reference.

   @param[in]  tAbsoluteTime seek to the absolute position(ms).
   @param[in]  bSeekToSync   When set to false, parser can seek to non sync frame
   @param[in]  nCurrPlayTime Current playback time.(-1 indicates time unknown)
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekAbsolutePosition() is called.
            bSeekToSync is only applicable if there is video track as for audio,
            every frame is a sync frame.
========================================================================== */

FileSourceStatus FileSourceHelper::SeekAbsolutePosition(const int64 tAbsoluteTime,
                                                        bool bSeekToSync,
                                                        int64 nCurrPlayTime)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  uint8 nRet = IsSeekDenied();
  if(!m_bEveryThingOK)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"SeekAbsolutePosition m_bEveryThingOK is FALSE!!");
    status = FILE_SOURCE_NOTAVAILABLE;
  }
  else if((!nRet) || (!tAbsoluteTime))
  {
    m_nSeekAbsoluteTime = tAbsoluteTime;
    m_bSeekToSync = bSeekToSync;
    m_nPlaybackTime = nCurrPlayTime;
    m_eSeekType = FILE_SOURCE_SEEK_ABSOLUTE;
    status = FILE_SOURCE_SUCCESS;
    if (m_bFSAsync)
    {
      /* Post the event to the seek file handler */
      MM_Signal_Set( m_pSeekFileSignal );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::SourceThread received SEEK_FILE_EVENT");
      HandleSeekFileCommand();
    }
  }
  else
  {
    m_fileSourceHelperCallBackFunc(FILE_SOURCE_SEEK_FAIL, m_pClientData);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "SeekAbsolutePosition Seek not allowed");
    status = FILE_SOURCE_FAIL;
  }
  return status;
}
/*! ======================================================================
   @brief      Reposition to an absolute position.

   @detail    This API will try to seek the track identified via trackid to
              given absolute time provided there is an I frame at that time.
              If there are no I frames in forward direction, forward seek
              request can fail. To allow parser to seek to non key frame,
              bSeekToSync can be set to false.

   @param[in]  trackid       Track-Id to specify which track to seek
   @param[in]  tAbsoluteTime Seek to the absolute position(ms).
   @param[in]  bSeekToSync   When set to false, parser can seek to non sync frame
   @param[in]  nCurrPlayTime Current playback time.(-1 indicates time unknown)

   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekAbsolutePosition() is called.
            bSeekToSync is only applicable if there is video track as for audio,
            every frame is a sync frame.
========================================================================== */

FileSourceStatus FileSourceHelper::SeekAbsolutePosition(int64 trackid,
                                                        const int64 tAbsoluteTime,
                                                        bool bSeekToSync,
                                                        int64 nCurrPlayTime)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  uint8 nRet = IsSeekDenied();
  if(!m_bEveryThingOK)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"SeekAbsolutePosition m_bEveryThingOK is FALSE!!");
    status = FILE_SOURCE_NOTAVAILABLE;
  }
  else if((!nRet) || (!tAbsoluteTime))
  {
    m_nSeekAbsoluteTime = tAbsoluteTime;
    m_bSeekToSync = bSeekToSync;
    m_nPlaybackTime = nCurrPlayTime;
    m_nTrackIdToSeek = trackid;
    m_eSeekType = FILE_SOURCE_SEEK_ABSOLUTE;
    status = FILE_SOURCE_SUCCESS;
    if (m_bFSAsync)
    {
      /* Post the event to the seek file handler */
      MM_Signal_Set( m_pSeekFileSignal );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::SourceThread received SEEK_FILE_EVENT");
      HandleSeekFileCommand();
    }
  }
  else
  {
    m_fileSourceHelperCallBackFunc(FILE_SOURCE_SEEK_FAIL, m_pClientData);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "SeekAbsolutePosition Seek not allowed");
    status = FILE_SOURCE_FAIL;
  }
  return status;
}
/*! ======================================================================
   @brief      Reposition to a relative Sync Sample/number of sync sample.

   @detail    The routine will seek/repositon to next sync sample based on the timestamp
              provided. The direction can be both positive and negative.

   @param[in]   currentPlaybacktime current playback time(ms) from the point of view of the User/Caller.
   @param[in]   numSync number of sync sample to jump. The value can be both positive and negative
               , specifiying the direction to search for the Sync Sample.
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekRelativeSyncPoint() is called.
========================================================================== */

FileSourceStatus FileSourceHelper::SeekRelativeSyncPoint( uint64 currentPlaybacktime, const int numSync)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  uint8 nRet = IsSeekDenied();
  if(!m_bEveryThingOK)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"SeekRelativeSyncPoint m_bEveryThingOK is FALSE!!");
    status = FILE_SOURCE_NOTAVAILABLE;
  }
  else if(!nRet)
  {
    m_nCurrentPlaybacktimeForSeek = currentPlaybacktime;
    m_nNumSync = numSync;
    m_eSeekType = FILE_SOURCE_SEEK_RELATIVE;
    status = FILE_SOURCE_SUCCESS;
    if (m_bFSAsync)
    {
      /* Post the event to the seek file handler */
      MM_Signal_Set( m_pSeekFileSignal );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::SourceThread received SEEK_FILE_EVENT");
      HandleSeekFileCommand();
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "SeekRelativeSyncPoint Seek not allowed");
    status = FILE_SOURCE_FAIL;
  }
  return status;
}

/*! ======================================================================
   @brief      Retrieves clip specifc DRM information

   @param[in/out]   pointer to retrieve the Clip specific DRM information. Memory should
          be allocated before the pointer is passed to this function.

   @note    It is expected that OpenFile() is called before GetClipDrmInfo() is called.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DIVX_DRM
FileSourceStatus FileSourceHelper::GetClipDrmInfo(ClipDrmInfoT* pDrmInfo)
#else
FileSourceStatus FileSourceHelper::GetClipDrmInfo(ClipDrmInfoT*)
#endif
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  if ((m_pVideoFileHandle) &&(m_pVideoFileHandle->isAviFileInstance()))
  {
    m_pVideoFileHandle->GetClipDrmInfo(pDrmInfo);
    status = FILE_SOURCE_SUCCESS;
  }
#endif
  return status;
}

/*! ======================================================================
   @brief   function to findout if the clip is DRM protected or not.

   @return    True if file is protected else returns false.

   @note    It is expected that OpenFile() is called before IsDRMProtection() is called.
========================================================================== */

bool FileSourceHelper::IsDRMProtection()
{
  bool status = false;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return status;
  }
  if ((m_pVideoFileHandle) &&(m_pVideoFileHandle->IsDRMProtection()))
  {
    status = true;
  }
  else if((m_pAudioFileHandle) && (m_pAudioFileHandle->IsDRMProtection()))
  {
    status = true;
  }
  return status;
}

/*!
   @brief   function to register DRM Decryption function

   @return  True if successful else returns false.

   @note    It is expected that OpenFile() is called before
            RegisterDRMDecryptMethod is called.
 */

 bool FileSourceHelper::RegisterDRMDecryptMethod(DRMDecryptMethodT pDecrypt,
                                                 void* pClientData)
 {
   bool status = false ;

   if (m_pVideoFileHandle)
   {
     status = m_pVideoFileHandle->RegisterDRMDecryptMethod(pDecrypt, pClientData);
   }
   if (m_pAudioFileHandle)
   {
     status = m_pAudioFileHandle->RegisterDRMDecryptMethod(pDecrypt, pClientData);
   }

   return status;
 }
/*!
  @brief   Function to get the device registration code.

  @return  True if successful in retrieving the code else returns false.

  @note    This API needs to be called in 2 step process.
           Call with NULL as first parameter and API will return the size needed for registration code.
           Allocate the memory and call again to get the registration code.
*/
bool FileSourceHelper::GetRegistrationCode(char* ptr,int* plength)
{
  bool status = false;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  QtvDivXDrmClient* drmclnt = MM_New_Args(QtvDivXDrmClient,());
  if(drmclnt)
  {
    status = drmclnt->GetRegistrationCode(ptr,plength);
    MM_Delete(drmclnt);
  }
#else
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::GetRegistrationCode ptr %p, len ptr %p",ptr, plength);
#endif
  return status;
}

/*! ======================================================================
  @brief Retrieve the wav codec data needed to configure WAV decoder.

  @param[in] id   Identifies the WAV track for which codec data needs to be retrieved
  @param[in,out] pBlock filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
========================================================================== */
bool FileSourceHelper::GetWavCodecData(uint32 id,WavFormatData* pCodecData)
{
  bool bRet = false;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return bRet;
  }
  if(m_pAudioFileHandle && pCodecData)
  {
    memset(pCodecData,0,sizeof(WavFormatData));
    pCodecData->channels = (uint16)m_pAudioFileHandle->GetNumAudioChannels(id);
    pCodecData->sample_rate = m_pAudioFileHandle->getTrackAudioSamplingFreq(id);
    pCodecData->channel_mask = m_pAudioFileHandle->GetAudioChannelMask(id);
    bRet = true;
  }
  return bRet;
}

/*! ======================================================================
  @brief Retrieve the wma codec data needed to configure WMA decoder.

  @param[in] id   Identifies the WMA track for which codec data needs to be retrieved
  @param[in,out] pBlock filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
========================================================================== */
bool FileSourceHelper::GetWMACodecData(uint32 id,WmaCodecData* pCodecData)
{
  bool bRet = false;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return bRet;
  }
#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  if(m_pAudioFileHandle && pCodecData)
  {
    memset(pCodecData,0,sizeof(WmaCodecData));
    pCodecData->nBitsPerSample =
    m_pAudioFileHandle->GetAudioBitsPerSample(id);
    pCodecData->nEncodeOpt =
    m_pAudioFileHandle->GetAudioEncoderOptions(id);
    pCodecData->nAdvEncodeOpt =
    m_pAudioFileHandle->GetAudioAdvancedEncodeOptions(id);
    pCodecData->nAdvEncodeOpt2 =
    m_pAudioFileHandle->GetAudioAdvancedEncodeOptions2(id);
    pCodecData->nChannelMask =
    m_pAudioFileHandle->GetAudioChannelMask(id);
    pCodecData->nVirtualPktSize =
    m_pAudioFileHandle->GetAudioVirtualPacketSize(id);
    pCodecData->nASFPacketSize =
    m_pAudioFileHandle->GetFixedAsfAudioPacketSize(id);
    pCodecData->nFormatTag = m_pAudioFileHandle->GetFormatTag(id);
    pCodecData->nBlockAlign = m_pAudioFileHandle->GetBlockAlign(id);
    bRet = true;
  }
#endif
  return bRet;
}

bool FileSourceHelper::GetAACCodecData(uint32 id,AacCodecData* pCodecData)
{
   bool bRet = false;
   if(!IS_FILESOURCE_STATUS_OK(m_eState))
   {
     return bRet;
   }
   uint32 nAACProfile = 0;
   uint32 nAACFormat = 0;
   if (m_pAudioFileHandle && pCodecData )
   {
      memset(pCodecData, 0, sizeof(AacCodecData));
      nAACProfile = m_pAudioFileHandle->GetAACAudioProfile(id);
      pCodecData->ucAACProfile = (uint8)nAACProfile;
      nAACFormat = m_pAudioFileHandle->GetAACAudioFormat(id);
      switch(nAACFormat)
      {
      case 0:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_UNKNWON;
         break;
      case 1:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_ADTS;
         break;
      case 2:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_ADIF;
         break;
      case 3:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_RAW;
         break;
      case 4:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_LOAS;
         break;
      default:
         pCodecData->eAACStreamFormat = FILE_SOURCE_AAC_FORMAT_UNKNWON;
         break;
      }
      bRet = true;
   }
   return (bRet);
}

bool FileSourceHelper::GetFlacCodecData(uint32 id,FlacFormatData* pCodecData)
{
   bool bRet = false;
   if(!IS_FILESOURCE_STATUS_OK(m_eState))
   {
     return bRet;
   }
   if (m_pAudioFileHandle)
   {
     bRet = m_pAudioFileHandle->GetFlacCodecData(id,pCodecData);
   }
   return (bRet);
}

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/*! ======================================================================
@brief Returns absolute file offset(in bytes) associated with time stamp 'pbtime'(in milliseconds).

  @param[in]  pbtime Timestamp(in milliseconds) of the sample that user wants to play/seek
  @param[out] offset Absolute file offset(in bytes) corresponding to 'pbtime'

  @return true if successful in retrieving the absolute offset(in bytes) else returns false

  @note When there are multiple tracks in a given clip(audio/video/text),API returns
   minimum absolute offset(in bytes)among audio/video/text tracks.
   API is valid only for non-fragmented clips.
========================================================================== */
bool FileSourceHelper::GetOffsetForTime(uint64 ullPBTime, uint64* pullOffset)
{
  bool   bRet         = false;
  uint64 ullReposTime = ullPBTime;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return bRet;
  }
  if(pullOffset)
  {
    /* If video track is present and selected, calculate the required offset
       to play till Playback time requested.
       If Video track is not present, then Parser will check Audio track
       instance and calculates the offset value with respect to audio. */
    if(m_pVideoFileHandle && m_nVideoTracks)
    {
      if(m_pVideoCriticalSection)
      {
        MM_CriticalSection_Enter(m_pVideoCriticalSection);
      }
      bRet  = m_pVideoFileHandle->GetOffsetForTime(ullPBTime, pullOffset,
                                                   m_videoSelectedTrackId,
                                                   m_currentPosTimeStamp,
                                                   ullReposTime);
      if(m_pVideoCriticalSection)
      {
        MM_CriticalSection_Leave(m_pVideoCriticalSection);
      }
    }
    //if there is no video track, check whether it has audio or not
    else if(m_pAudioFileHandle && m_nAudioTracks && m_pAudioCriticalSection)
    {
      MM_CriticalSection_Enter(m_pAudioCriticalSection);
      /***********************************************************************/
      /* Check File format type for given Track ID. If File format is audio  */
      /* only container format, then get file size and duration from the     */
      /* corresponding parser and calculate the approximate offset value     */
      /* to be downloaded to play till the duration requested.               */
      /* For A-V containers, corresponding parsers will have implementations.*/
      /***********************************************************************/
      FileSourceFileFormat eFileFormat = FILE_SOURCE_UNKNOWN;
      m_pAudioFileHandle->GetFileFormat(eFileFormat);
      switch(eFileFormat)
      {
        case FILE_SOURCE_AAC:
        case FILE_SOURCE_AC3:
        case FILE_SOURCE_AMR_NB:
        case FILE_SOURCE_AMR_WB:
        case FILE_SOURCE_DTS:
        case FILE_SOURCE_FLV:
        case FILE_SOURCE_MP3:
        case FILE_SOURCE_OGG:
        case FILE_SOURCE_QCP:
        case FILE_SOURCE_WAV:
        {
          uint64 ullFileSize = m_pAudioFileHandle->GetFileSize();
          uint64 ullDuration = m_pAudioFileHandle->getMovieDuration();
          uint32 ulTimeScale = m_pAudioFileHandle->getMovieTimescale();

          //! This API converts file duration into milli-sec units.
          //! Client gave PB time in milli-sec units.
          ullDuration = (ullDuration * MILLISEC_TIMESCALE_UNIT) /
                        (ulTimeScale);
          if ((ullFileSize) && (MAX_FILE_SIZE != ullFileSize) && (ullDuration))
          {
            *pullOffset = (ullPBTime * ullDuration) / ullFileSize;
            bRet        = true;
          }
        }
        break;
        default:
        {
          bRet = false;
        }
        break;
      }
      if (false == bRet)
      {
        bRet  = m_pAudioFileHandle->GetOffsetForTime(ullPBTime, pullOffset,
                                                     m_audioSelectedTrackId,
                                                     m_currentPosTimeStamp,
                                                     ullReposTime);
      }
      MM_CriticalSection_Leave(m_pAudioCriticalSection);
    }
    //if there is no audio track as well, check whether it has text or not
    else if(m_pTextFileHandle && m_nTextTracks)
    {
      if(m_pTextCriticalSection)
      {
        MM_CriticalSection_Enter(m_pTextCriticalSection);
      }
      bRet  = m_pTextFileHandle->GetOffsetForTime(ullPBTime, pullOffset,
                                                  m_textSelectedTrackId,
                                                  m_currentPosTimeStamp,
                                                  ullReposTime);
      if(m_pTextCriticalSection)
      {
        MM_CriticalSection_Leave(m_pTextCriticalSection);
      }
    }
  }
  return bRet;
}
#endif

/*! ======================================================================
  @brief Returns absolute file offset(in bytes)
         corresponding to current sample being retrieved via getNextMediaSample.

  @param[in]  trackid TrackId identifying the media track
  @param[out] bError  Indicates if error occured while retrieving file offset.

  @note       bError is set to true when error occurs otherwise, set to false.
  @return Returns absolute file offset(in bytes) that corresponds to
          current sample retrieved via getNextMediaSample.
========================================================================== */
uint64 FileSourceHelper::GetLastRetrievedSampleOffset(uint32 trackid,
                                                      bool* bError)
{
  uint64 soffset = 0;
  if(bError)
  {
    *bError = true;
    if(!IS_FILESOURCE_STATUS_OK(m_eState))
    {
      return soffset;
    }
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    FileSourceMjMediaType majorType;
    majorType = GetFileSourceMajorType(trackid);
    //Make sure track-id is the valid trackid
    switch (majorType)
    {
      //consult corresponding filebase handle
      case FILE_SOURCE_MJ_TYPE_AUDIO:
        if(m_pAudioFileHandle)
        {
          soffset = m_pAudioFileHandle->GetLastRetrievedSampleOffset(trackid);
          *bError = false;
        }
        break;
      case FILE_SOURCE_MJ_TYPE_VIDEO:
        if(m_pVideoFileHandle)
        {
          soffset = m_pVideoFileHandle->GetLastRetrievedSampleOffset(trackid);
          *bError = false;
        }
        break;
      case FILE_SOURCE_MJ_TYPE_TEXT:
        if(m_pTextFileHandle)
        {
          soffset = m_pTextFileHandle->GetLastRetrievedSampleOffset(trackid);
          *bError = false;
        }
        break;
      default:
        break;
    }
#endif
  }
  return soffset;
}

/*! ======================================================================
  @brief Returns total number of views associated with given trackid.
         Default value is always 1 for any given track unless the track has MVC(MultiViewCoding).

  @param[in]  trackid TrackId identifying the media track
  @return     Returns total number of views for given trackid
  @note       It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */
uint16 FileSourceHelper::GetNumberOfViews(uint32 trackid)
{
  uint16 nviews = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return nviews;
  }
  FileSourceMjMediaType majorType;
  majorType = GetFileSourceMajorType(trackid);
  //Make sure track-id is the valid trackid
  switch (majorType)
  {
    //consult corresponding filebase handle
    case FILE_SOURCE_MJ_TYPE_VIDEO:
      if(m_pVideoFileHandle)
      {
        FS_VIDEO_PARAM_H264TYPE sH264Info;
        memset(&sH264Info, 0, sizeof(FS_VIDEO_PARAM_H264TYPE));
        if( PARSER_ErrorNone == m_pVideoFileHandle->GetStreamParameter(trackid,
                                                   FS_IndexParamVideoH264,
                                                   &sH264Info))
        {
          nviews = sH264Info.ulNumberOfViews;
        }
      }
      break;
    default:
      break;
  }
  return nviews;
}

/*! ======================================================================
    @brief   Returns current source error, if any.

    @return  error code status of underlying file source

    @note    It is expected that OpenFile() is called before GetFileError() is called.
========================================================================== */
FileSourceParserError FileSourceHelper::GetFileError()
{
  FileSourceParserError ret = FILE_SOURCE_PARSER_UNKNOWN_ERROR;
  PARSER_ERRORTYPE status = PARSER_ErrorDefault;
  if(m_pVideoFileHandle)
  {
    status = m_pVideoFileHandle->GetFileError();
  }
  else if(m_pAudioFileHandle)
  {
    status = m_pAudioFileHandle->GetFileError();
  }
  else if(m_pTextFileHandle)
  {
    status = m_pTextFileHandle->GetFileError();
  }
  switch(status)
  {
    case PARSER_ErrorNone:
      ret = FILE_SOURCE_PARSER_OK;
    break;
    case PARSER_ErrorDefault:
      ret = FILE_SOURCE_PARSER_UNKNOWN_ERROR;
    break;
    case PARSER_ErrorZeroTracks:
      ret = FILE_SOURCE_PARSER_NO_MEDIA_TRACKS;
    break;
    case PARSER_ErrorDRMAuthorization:
      ret = FILE_SOURCE_PARSER_DRM_NO_AUTHORIZATION;
    break;
    case PARSER_ErrorDRMDeviceNotRegistered:
      ret = FILE_SOURCE_PARSER_DRM_DEVICE_NOT_REGISTERED;
    break;
    case PARSER_ErrorDRMRentalCountExpired:
      ret = FILE_SOURCE_PARSER_DRM_RENTAL_COUNT_EXPIRED;
    break;
    case PARSER_ErrorDRMMemAllocFail:
      ret = FILE_SOURCE_PARSER_DRM_OUT_OF_MEMORY;
    break;
    case PARSER_ErrorDRMPlaybackError:
      ret = FILE_SOURCE_PARSER_DRM_PLAYBACK_FAIL;
    break;
    default:
      ret = FILE_SOURCE_PARSER_PARSE_ERROR;
    break;
  }
  return ret;
}

/*! ======================================================================
  @brief  local function to check if Seek is allowed in the clip

  @param[in]  none
  @return     TRUE or FALSE
  @note   It is expected that OpenFile() is called before getIndexInTrackIdInfo() is called.
========================================================================== */

uint8 FileSourceHelper::IsSeekDenied()
{
  uint8 audioRet = 0;
  uint8 videoRet = 0;
  uint8 textRet = 0;
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    return true;
  }

  //Checking if parsing was successful and the also if media handles are valid
  if(m_nAudioTracks && m_pAudioFileHandle )
  {
    audioRet = m_pAudioFileHandle->randomAccessDenied();
  }
  if(m_nVideoTracks && m_pVideoFileHandle )
  {
    videoRet = m_pVideoFileHandle->randomAccessDenied();
  }
  if(m_nTextTracks && m_pTextFileHandle )
  {
    textRet = m_pTextFileHandle->randomAccessDenied();
  }
  return (audioRet || videoRet || textRet);
}

/*! ======================================================================
  @brief   Initialize all members of FileSource.

  @return  none
========================================================================== */

void FileSourceHelper::BaseInitData()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::BaseInitData");

  //Initialize File handles
  m_pAudioFileHandle = NULL;
  m_pVideoFileHandle = NULL;
  m_pTextFileHandle = NULL;

  // Initialize the number of audio tracks present in the clip
  m_nAudioTracks = 0;
  m_nVideoTracks = 0;
  m_nTextTracks =0;

  // Initialize selected audio/video/text tracks
  m_audioSelectedTrackId = -1;
  m_videoSelectedTrackId = -1;
  m_textSelectedTrackId  = -1;

  // Initialize timestamp of the current sample accessed
  m_currentPosTimeStamp = 0;
  // Initialize timestamp of the last sample in the clip
  m_stopPosTimeStamp = 0;
  m_nTrackIdInfoIndexToUse = 0;
  m_bLookForCodecHdr = true;
  m_bFSAsync = true;

  // Initialize the trackIfInfo
  for ( int i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
  {
    m_trackIdInfo[i].id = 0;
    m_trackIdInfo[i].selected = false;
    m_trackIdInfo[i].majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
  }

  // Initialize the medtaTrackInfo
  for ( int i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
  {
    (void) memset(&m_audioTrackInfo[i], 0, sizeof(FileSourceAudioInfo) );
    (void) memset(&m_videoTrackInfo[i], 0, sizeof(FileSourceVideoInfo) );
    (void) memset(&m_textTrackInfo[i],  0, sizeof(FileSourceTextInfo) );
  }
  memset(&sAC3HdrInfo, 0, sizeof(AC3HeaderInfo));
}

/*! ======================================================================
  @brief        retrieve the information about the audio track

  @detail        This function retrieves the information about the audio track
        from fileBase.

  @return      none
  @note   It is expected that fillAudioTrackInfo() is called from/after OpenFile().
========================================================================== */

void FileSourceHelper::FillAudioTrackInfo()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::fillAudioTrackInfo");
  //Make sure we have valid file handle and initial parsing was done successfully
  if (m_pAudioFileHandle && m_pAudioFileHandle->FileSuccess() )
  {
    uint32 numTracks = m_pAudioFileHandle->getNumTracks();
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
           "FileSource::fillAudioTrackInfo numTracks in file %lu",numTracks);
    uint32* idList = NULL;
    int index =0;
    //even though there are more tracks than what filesource supports,
    //we should allocate memory for all tracks otherwise
    //getTrackWholeIDList will cause buffer overflow in respective parser while filling in track id.
    if ( numTracks > FILE_SOURCE_MAX_NUM_TRACKS )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Clip has more tracks than supported");
    }

    if (numTracks)
    {
      idList = MM_New_Array(uint32,(numTracks));
      if (idList)
      {
        (void)m_pAudioFileHandle->getTrackWholeIDList(idList);
      }
      //We should take MIN between number of tracks found in file, max. tracks supported in FileSource
      for (uint32 itr = 0; (itr < FILESOURCE_MIN(numTracks,FILE_SOURCE_MAX_NUM_TRACKS)) && (idList); itr++)
      {
        uint32 trackId = idList[itr];
        uint8 codecType = m_pAudioFileHandle->getTrackOTIType(trackId);
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "FileSource::fillAudioTrackInfo codecType ret from getTrackOTIType %d",
        codecType);

        //make sure there is room to store track information in m_trackIdInfo
        if( (IS_AUDIO_CODEC(codecType)) && (m_nTrackIdInfoIndexToUse < FILE_SOURCE_MAX_NUM_TRACKS) )
        {
          // Fill in the trackIdInfoType
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].id = trackId;
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].majorType = FILE_SOURCE_MJ_TYPE_AUDIO;

          // Fill in the FileSourceAudioInfo
          m_audioTrackInfo[index].id = trackId;
          m_audioTrackInfo[index].audioCodec =  MapCodecToMinorType(codecType);
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                       "FILE_SOURCE_MN_TYPE [%d] for TRACK_ID [%lu]",
                       m_audioTrackInfo[index].audioCodec, trackId);

          // number of audio channels
          m_audioTrackInfo[index].numChannels = m_pAudioFileHandle->GetNumAudioChannels(trackId);;

          // time scale of the track
          uint32 ulTimeScale = m_pAudioFileHandle->getTrackMediaTimescale(trackId);

          // total duration for the track in microseconds
          if (ulTimeScale)
          {
            uint64 ullDur = m_pAudioFileHandle->getTrackMediaDuration(trackId);
            ullDur = (MICROSEC_TIMESCALE_UNIT * ullDur) / ulTimeScale;
            m_audioTrackInfo[index].duration  = ullDur;
            m_audioTrackInfo[index].timeScale = MICROSEC_TIMESCALE_UNIT;
          }

          // bit rate
          m_audioTrackInfo[index].bitRate =  m_pAudioFileHandle->getTrackAverageBitrate(trackId);

          // min bit rate
          m_audioTrackInfo[index].minBitRate =  m_pAudioFileHandle->getTrackMinBitrate(trackId);

          // max bit rate
          m_audioTrackInfo[index].maxBitRate =  m_pAudioFileHandle->getTrackMaxBitrate(trackId);

          // audio sampling rate
          m_audioTrackInfo[index].samplingRate =  m_pAudioFileHandle->getTrackAudioSamplingFreq(trackId);
          // valid bits per sample
          m_audioTrackInfo[index].nBitsPerSample =  m_pAudioFileHandle->GetAudioBitsPerSample(trackId);

          /* Encoder and Padding delay are useful for gap-less playback. This
             data will be available in Metadata section. Currently only AAC,
             MP3 and MP4 Parsers only has this info.
             Metadata is available in Hex string format. Convert string to
             decimal format before using. */
          wchar_t ucDelayString[32];
          uint32 ulSize     = 32;
          uint64 ullDelay   = 0;
          char*  pStringEnd = NULL;
          memset(ucDelayString, 0, sizeof(ucDelayString));
          (void)m_pAudioFileHandle->GetClipMetaData((wchar_t*)ucDelayString,
                                                    &ulSize,
                                                    FILE_SOURCE_MD_ENC_DELAY);
          if (ulSize)
          {
            ullDelay = strtoul((const char*)ucDelayString, &pStringEnd, 16);
          }
          m_audioTrackInfo[index].nEncoderDelay = (uint32)ullDelay;

          (void)m_pAudioFileHandle->GetClipMetaData(
                                                (wchar_t*)ucDelayString,
                                                &ulSize,
                                                FILE_SOURCE_MD_PADDING_DELAY);
          if (ulSize)
          {
            ullDelay = strtoul((const char*)ucDelayString, &pStringEnd, 16);
          }
          m_audioTrackInfo[index].nPaddingDelay = (uint32)ullDelay;

          (void)m_pAudioFileHandle->GetClipMetaData(
                                                (wchar_t*)ucDelayString,
                                                &ulSize,
                                                FILE_SOURCE_MD_SEEK_PREROLL_DELAY);
          if (ulSize)
          {
            ullDelay = strtoul((const char*)ucDelayString, &pStringEnd, 16);
          }
          m_audioTrackInfo[index].nSeekPrerollDelay = ullDelay;
          //ac3/ec3 handling
          //update number of independent stream
          if( ( FILE_SOURCE_MN_TYPE_EAC3 == \
                  m_audioTrackInfo[index].audioCodec ) ||
              ( FILE_SOURCE_MN_TYPE_EAC3_JOC == \
                  m_audioTrackInfo[index].audioCodec ) ||
              ( FILE_SOURCE_MN_TYPE_AC3  == \
                  m_audioTrackInfo[index].audioCodec ) )
          {
            FS_AUDIO_PARAM_AC3TYPE sDDInfo;
            memset(&sDDInfo, 0, sizeof(FS_AUDIO_PARAM_AC3TYPE));
            //By default parser will report default stream id i.e. first
            //independent sub stream. If client want to select different stream
            //than need to fetch stream information separately.
            //In case of AC3 there will only one independent stream while EAC3
            //audio can have 8 independent sub stream.
            sDDInfo.ucProgramID = 1;
            if( PARSER_ErrorNone == \
                m_pAudioFileHandle->GetStreamParameter(trackId,
                                                       FS_IndexParamAudioAC3,
                                                       &sDDInfo))
            {
              m_audioTrackInfo[index].ucNoOfIndSubStrm = sDDInfo.ucNumOfIndSubs;
              m_audioTrackInfo[index].numChannels = sDDInfo.usNumChannels;
              MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_LOW,
                "AC3/EC3: IndSub Stream = %d NoOfChannels = %d,ACMOD = %d BSMOD = %d",
                sDDInfo.ucNumOfIndSubs, sDDInfo.usNumChannels,
                sDDInfo.ucAudioCodingMode, sDDInfo.ucBitStreamMode);
            }
          }
          m_nAudioTracks++;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "FileSource::fillAudioTrackInfo m_nAudioTracks %lu",m_nAudioTracks);
          index++;
          m_nTrackIdInfoIndexToUse++;
        }
      }
      if(idList)
      {
        MM_Delete_Array(idList);
      }
    }//if (numTracks)
  }//if (m_pAudioFileHandle && m_pAudioFileHandle->FileSuccess() )
}

/*! ======================================================================
  @brief        retrieve the information about the video track

  @detail        This function retrieves the information about the audio track
        from fileBase.

  @return      none
  @note   It is expected that fillVideoTrackInfo() is called from/after OpenFile().
========================================================================== */

void FileSourceHelper::FillVideoTrackInfo()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::fillVideoTrackInfo");
  //Make sure we have valid file handle and initial parsing was done successfully
  if (m_pVideoFileHandle && m_pVideoFileHandle->FileSuccess() )
  {
    uint32 numTracks = m_pVideoFileHandle->getNumTracks();
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
           "FileSource::fillVideoTrackInfo numTracks in file %lu",numTracks);
    uint32* idList = NULL;
    int index =0;

    //even though there are more tracks than what filesource supports,
    //we should allocate memory for all tracks otherwise
    //getTrackWholeIDList will cause buffer overflow in respective parser while filling in track id.
    if ( numTracks > FILE_SOURCE_MAX_NUM_TRACKS )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Clip has more tracks than supported");
    }

    if (numTracks)
    {
      idList = MM_New_Array(uint32,(numTracks));
      if (idList)
      {
        (void)m_pVideoFileHandle->getTrackWholeIDList(idList);
      }
      //We should take MIN between number of tracks found in file, max. tracks supported in FileSource
      for (uint32 itr = 0; (itr < FILESOURCE_MIN(numTracks,FILE_SOURCE_MAX_NUM_TRACKS)) && (idList); itr++)
      {
        uint32 trackId = idList[itr];
        uint8 codecType = m_pVideoFileHandle->getTrackOTIType(trackId);

        //make sure there is room to store track information in m_trackIdInfo
        if( (IS_VIDEO_CODEC(codecType)) && (m_nTrackIdInfoIndexToUse < FILE_SOURCE_MAX_NUM_TRACKS) )
        {
          // Fill in the trackIdInfoType
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].id = trackId;
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].majorType = FILE_SOURCE_MJ_TYPE_VIDEO;

          // Fill in the FileSourceVideoInfo
          m_videoTrackInfo[index].id = trackId;
          m_videoTrackInfo[index].videoCodec =  MapCodecToMinorType(codecType);
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                       "FILE_SOURCE_MN_TYPE [%d] for TRACK_ID [%lu]",
                       m_videoTrackInfo[index].videoCodec, trackId);

          // subdivision of level
          m_videoTrackInfo[index].layer = 1;

          // time scale of the track
          uint32 ullTimeScale = m_pVideoFileHandle->getTrackMediaTimescale(trackId);

          // total duration for the track in microseconds
          if (ullTimeScale)
          {
            uint64 ullDur = m_pVideoFileHandle->getTrackMediaDuration(trackId);
            ullDur = (MICROSEC_TIMESCALE_UNIT * ullDur) / ullTimeScale;
            m_videoTrackInfo[index].duration  = ullDur;
            m_videoTrackInfo[index].timeScale = MICROSEC_TIMESCALE_UNIT;
          }

          // bit rate
          m_videoTrackInfo[index].bitRate =  m_pVideoFileHandle->getTrackAverageBitrate(trackId);

          // frames per second
          m_videoTrackInfo[index].frameRate =  m_pVideoFileHandle->getTrackVideoFrameRate(trackId);

          // frame width in pixels
          m_videoTrackInfo[index].frameWidth =  m_pVideoFileHandle->getTrackVideoFrameWidth(trackId);

          //! frame height in pixels
          m_videoTrackInfo[index].frameHeight =  m_pVideoFileHandle->getTrackVideoFrameHeight(trackId);

          //! rotation angle in degrees
          m_videoTrackInfo[index].ulRotationDegrees =  m_pVideoFileHandle->getRotationDegrees(trackId);
          m_nVideoTracks++;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "FileSource::fillVideoTrackInfo m_nVideoTracks %lu",m_nVideoTracks);
          index++;
          m_nTrackIdInfoIndexToUse++;
        }
      }
      if(idList)
      {
        MM_Delete_Array(idList);
      }
    }//if (numTracks)
  }//if (m_pVideoFileHandle && m_pVideoFileHandle->FileSuccess() )
}

/*! ======================================================================
  @brief        retrieve the information about the text track

  @detail        This function retrieves the information about the audio track
        from fileBase.

  @return      none
  @note   It is expected that fillTextTrackInfo() is called from/after OpenFile().
========================================================================== */

void FileSourceHelper::FillTextTrackInfo()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::fillTextTrackInfo");
  //Make sure we have valid file handle and initial parsing was done successfully
  if (m_pTextFileHandle && m_pTextFileHandle->FileSuccess() )
  {
    uint32 numTracks = m_pTextFileHandle->getNumTracks();
    uint32* idList = NULL;
    int index =0;

    //even though there are more tracks than what filesource supports,
    //we should allocate memory for all tracks otherwise
    //getTrackWholeIDList will cause buffer overflow in respective parser while filling in track id.
    if ( numTracks > FILE_SOURCE_MAX_NUM_TRACKS )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "Clip has more tracks than supported");
    }

    if (numTracks)
    {
      FileSourceTextInfo sTextTrackInfo;
      idList = MM_New_Array(uint32,(numTracks));
      if (idList)
      {
        (void)m_pTextFileHandle->getTrackWholeIDList(idList);
      }
      //We should take MIN between number of tracks found in file, max. tracks supported in FileSource
      for (uint32 itr = 0; (itr < FILESOURCE_MIN(numTracks,FILE_SOURCE_MAX_NUM_TRACKS)) && (idList); itr++)
      {
        memset(&sTextTrackInfo, 0x0, sizeof(FileSourceTextInfo));
        uint32 trackId = idList[itr];
        uint8 codecType = m_pTextFileHandle->getTrackOTIType(trackId);

        //make sure there is room to store track information in m_trackIdInfo
        if( (IS_TEXT_CODEC(codecType)) && (m_nTrackIdInfoIndexToUse < FILE_SOURCE_MAX_NUM_TRACKS) )
        {
          // Fill in the trackIdInfoType
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].id = trackId;
          m_trackIdInfo[m_nTrackIdInfoIndexToUse].majorType = FILE_SOURCE_MJ_TYPE_TEXT;

          // Fill in the FileSourceTextInfo
          m_textTrackInfo[index].id = trackId;
          m_textTrackInfo[index].textCodec =  MapCodecToMinorType(codecType);
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                       "FILE_SOURCE_MN_TYPE [%d] for TRACK_ID [%lu]",
                       m_textTrackInfo[index].textCodec, trackId);

          // subdivision of level
          m_textTrackInfo[index].layer = 1;

          // time scale of the track
          uint32 ulTimeScale = m_pTextFileHandle->getTrackMediaTimescale(trackId);

          // total duration for the track in microseconds
          if (ulTimeScale)
          {
            uint64 ullDur = m_pTextFileHandle->getTrackMediaDuration(trackId);
            ullDur = (MICROSEC_TIMESCALE_UNIT * ullDur) / ulTimeScale;
            m_textTrackInfo[index].duration  = ullDur;
            m_textTrackInfo[index].timeScale = MICROSEC_TIMESCALE_UNIT;
          }
          // get media track information
          m_pTextFileHandle->GetStreamParameter(trackId,
                                                FS_IndexParamOtherMediaTrackInfo,
                                                &sTextTrackInfo);

          // frame width in pixels
          m_textTrackInfo[index].frameWidth = sTextTrackInfo.frameWidth;
          // frame height in pixels
          m_textTrackInfo[index].frameHeight = sTextTrackInfo.frameHeight;

          m_nTextTracks++;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "FileSource::fillTextTrackInfo m_nTextTracks %lu",m_nTextTracks);
          index++;
          m_nTrackIdInfoIndexToUse++;
        }
      }
      if(idList)
      {
        MM_Delete_Array(idList);
      }
    }//if (numTracks)
  }//if (m_pTextFileHandle && m_pTextFileHandle->FileSuccess() )
}

/*! ======================================================================
  @brief  select default audio/video/text tracks

  @detail select the first media (audio or video or text) track as the default
      media track

  @return none

  @note   It is expected that OpenFile() is called before selectDefaultTracks()
          is called.
========================================================================== */

void FileSourceHelper::SelectDefaultTracks()
{
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
  "FileSource::selectDefaultTracks m_audioSelectedTrackId %d \
  m_videoSelectedTrackId %d m_textSelectedTrackId %d",
  m_audioSelectedTrackId, m_videoSelectedTrackId, m_textSelectedTrackId);
  if ( (m_audioSelectedTrackId < 0) && m_nAudioTracks)
  {
    //Select the first Audio track
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
    "FileSource::selectDefaultTracks m_audioTrackInfo[0].id %lu",
    m_audioTrackInfo[0].id);
    SetSelectedTrackID(m_audioTrackInfo[0].id);
  }

  if ( (m_videoSelectedTrackId < 0) && m_nVideoTracks)
  {
    //Select the first Video track
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
    "FileSource::selectDefaultTracks m_videoTrackInfo[0].id %lu",
    m_videoTrackInfo[0].id);
    SetSelectedTrackID(m_videoTrackInfo[0].id);
  }

  if ( (m_textSelectedTrackId < 0) && m_nTextTracks)
  {
    //Select the first Text track
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
    "FileSource::selectDefaultTracks m_textTrackInfo[0].id %lu",
    m_textTrackInfo[0].id);
    SetSelectedTrackID(m_textTrackInfo[0].id);
  }
}

/*! ======================================================================
  @brief      method returns Major Type for the Track.

  @param[in]   id    track ID
  @return      major type.

  @note    It is expected that OpenFile() is called before getFileSourceMajorType() is called.
========================================================================== */

FileSourceMjMediaType FileSourceHelper::GetFileSourceMajorType(uint32 id)
{
  FileSourceMjMediaType majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
  bool foundFlag = false;
  uint32 i = 0;

  // search for the track in the audio tracks and retrieve the major.
  while ((!foundFlag)&&(i < m_nAudioTracks))
  {
    if ( id == m_audioTrackInfo[i++].id)
    {
      majorType = FILE_SOURCE_MJ_TYPE_AUDIO;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::getFileSourceMajorType id %lu MAJOR is AUDIO",id);
      foundFlag = true;
    }
  }

  // search for the track in the video tracks and retrieve the major
  i = 0;
  while ((!foundFlag)&&(i < m_nVideoTracks))
  {
    if ( id == m_videoTrackInfo[i++].id)
    {
      majorType = FILE_SOURCE_MJ_TYPE_VIDEO;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::getFileSourceMajorType id %lu MAJOR is VIDEO",id);
      foundFlag = true;
    }
  }

  // search for the track in the text tracks and retrieve the major
  i = 0;
  while ((!foundFlag)&&(i < m_nTextTracks))
  {
    if ( id == m_textTrackInfo[i++].id)
    {
      majorType = FILE_SOURCE_MJ_TYPE_TEXT;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::getFileSourceMajorType id %lu MAJOR is TEXT",id);
      foundFlag = true;
    }
  }

  return majorType;
}

/*! ======================================================================
  @brief      method returns Minor Type for the track.

  @param[in]   id track ID
  @return      minor type.

  @note    It is expected that OpenFile() is called before getFileSourceMinorType() is called.
========================================================================== */


FileSourceMnMediaType FileSourceHelper::GetFileSourceMinorType(uint32 id)
{
  int index = -1;
  FileSourceMnMediaType trackMinorType = FILE_SOURCE_MN_TYPE_UNKNOWN;

  // get the index of the track from mediaTrackInfo
  index = GetIndexInMediaTrackInfo(id);

  if ( index != -1)
  {
    switch (GetFileSourceMajorType(id))
    {
    case FILE_SOURCE_MJ_TYPE_AUDIO:
      trackMinorType = m_audioTrackInfo[index].audioCodec;
      break;
    case FILE_SOURCE_MJ_TYPE_VIDEO:
      trackMinorType = m_videoTrackInfo[index].videoCodec;
      break;
    case FILE_SOURCE_MJ_TYPE_TEXT:
      trackMinorType = m_textTrackInfo[index].textCodec;
      break;
    default:
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "FileSource::getFileSourceMinorType invliad minor type id %lu",id);
      }  // do nothing */
      break;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
               "FileSource::getFileSourceMinorType id %lu MINOR is %d",
               id,(int)trackMinorType);
  return trackMinorType;
}

/*! ======================================================================
  @brief      maps codec to minor media type.

  @param[in]   codec type of coded ( audio or video or text)
  @return      minor type.

  @note    It is expected that OpenFile() is called before mapCodecToMinorType() is called.
========================================================================== */

FileSourceMnMediaType FileSourceHelper::MapCodecToMinorType(uint8 codec)
{
  FileSourceMnMediaType trackMinorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
  switch (codec)
  {
  // Audio Codecs
  case EVRC_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_EVRC;
    break;
  case PUREVOICE_AUDIO:
  case PUREVOICE_AUDIO_2:
  case QCP_QLCM_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_QCELP;
    break;
  case MPEG2_AAC_LC:
  case MPEG4_AUDIO :
    trackMinorType = FILE_SOURCE_MN_TYPE_AAC;
    break;
  case AAC_ADTS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AAC_ADTS;
    break;
  case AAC_ADIF_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AAC_ADIF;
    break;
  case AAC_LOAS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AAC_LOAS;
    break;
  case AMR_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_GSM_AMR;
    break;
  case WM_AUDIO :
    trackMinorType = FILE_SOURCE_MN_TYPE_WMA;
    break;
  case WM_SPEECH :
    trackMinorType = FILE_SOURCE_MN_TYPE_WM_SPEECH;
    break;
  case WM_PRO_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_WMA_PRO;
    break;
  case WM_LOSSLESS:
    trackMinorType = FILE_SOURCE_MN_TYPE_WMA_LOSSLESS;
    break;
  case MP3_AUDIO:
  case NONMP4_MP3_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MP3;
    break;
  case MP2_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MP2;
    break;
  case AMR_WB_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AMR_WB;
    break;
  case AMR_WB_PLUS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AMR_WB_PLUS;
    break;
  case EVRC_WB_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_EVRC_WB;
    break;
  case EVRC_B_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_EVRC_B;
    break;
  case MPEG4_AUDIO_BSAC:
    trackMinorType = FILE_SOURCE_MN_TYPE_BSAC;
    break;
  case QCP_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_QCP;
    break;
  case NONMP4_AAC_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_NONMP4_AAC;
    break;
  case NONMP4_AMR_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_NONMP4_AMR;
    break;
  case MIDI_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MIDI;
    break;
  case DTS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_DTS;
    break;
  case AC3_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_AC3;
    break;
  case EAC3_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_EAC3;
    break;
  case EAC3_JOC_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_EAC3_JOC;
    break;
  case PCM_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_PCM;
    break;
  case G711_ALAW_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_G711_ALAW;
    break;
  case G711_MULAW_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_G711_MULAW;
    break;
  case G721_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_G721;
    break;
  case G723_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_G723;
    break;
  case VORBIS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_VORBIS;
    break;
  case FLAC_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_FLAC;
    break;
  case GSM_FR_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_GSM_FR;
    break;
  case OPUS_AUDIO:
    trackMinorType = FILE_SOURCE_MN_TYPE_OPUS;
    break;
      // Video codecs
  case MPEG4_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MPEG4;
    break;
  case NONSTD_MPEG4_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_NONSTD_MPEG4;
    break;
  case MPEG4_IMAGE:
    trackMinorType = FILE_SOURCE_MN_TYPE_STILL_IMAGE;
    break;
  case H263_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_H263;
    break;
  case H264_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_H264;
    break;
  case HEVC_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_HEVC;
    break;
  case VP6F_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_VP6F;
    break;
  case WM_VIDEO_7:
    trackMinorType = FILE_SOURCE_MN_TYPE_WMV1;
    break;
  case WM_VIDEO_8:
    trackMinorType = FILE_SOURCE_MN_TYPE_WMV2;
    break;
  case WM_VIDEO_9:
    trackMinorType = FILE_SOURCE_MN_TYPE_WMV3;
    break;
  case VC1_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_VC1;
    break;
#ifdef FEATURE_DIVX_311_ENABLE
  case DIVX311_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_DIVX311;
    break;
#endif
#ifdef FEATURE_FILESOURCE_AVI_DIVX_PARSER
  case DIVX40_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_DIVX40;
    break;
  case DIVX50_60_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_DIVX50_60;
    break;
#endif
#ifdef FEATURE_FILESOURCE_MPEG2_PARSER
    case MPEG2_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MPEG2;
    break;
#endif
  case MPEG1_VIDEO:
    //We can mark MPEG1 video as MPEG2 video itself from parser,
    // as most of the MPEG2 decoder will able to handle MPEG1 video.
    //So no need to explicitly set MPEG1 video from parser.
    trackMinorType = FILE_SOURCE_MN_TYPE_MPEG2;
    break;
  case THEORA_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_THEORA;
    break;
  case SPARK_VIDEO:
     trackMinorType = FILE_SOURCE_MN_TYPE_SORENSON_SPARK;
     break;
  case VP8F_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_VP8F;
    break;
  case VP9_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_VP9;
    break;
  case REAL_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_REAL;
    break;
  case MJPEG_VIDEO:
    trackMinorType = FILE_SOURCE_MN_TYPE_MJPEG;
    break;
    // Text Codecs
  case TIMED_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_TIMED_TEXT;
    break;
  case AVI_SIMPLE_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_GENERIC_TEXT;
    break;
  case AVI_BITMAP_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_XSUB_BITMAP_TEXT;
    break;
  case SMPTE_TIMED_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT;
    break;
  case BITMAP_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_BITMAP_TEXT;
    break;
  case UTF8_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_UTF8_TEXT;
    break;
  case USF_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_USF_TEXT;
    break;
  case SSA_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_SSA_TEXT;
    break;
  case ASS_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_ASS_TEXT;
    break;
  case VOBSUB_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_VOBSUB_TEXT;
    break;
  case KARAOKE_TEXT:
    trackMinorType = FILE_SOURCE_MN_TYPE_KARAOKE_TEXT;
    break;

  default:
    ; /* do nothing */
  }
  return trackMinorType;
}

/*! ======================================================================
   @brief    Maps the Filebase library errors to FileSource Errors
   @param[in]  retError    Error code obtained from FileBase Library
   @param[out] status   Output FileSource Error code.
   @note    It is expected that OpenFile() is called before MapFileBaseError2FileSourceError() is called.
========================================================================== */
void FileSourceHelper::MapParserError2FileSourceStatus(PARSER_ERRORTYPE retError, FileSourceMediaStatus &status)
{
  switch(retError)
  {
    case PARSER_ErrorNone          :
    case PARSER_ErrorZeroSampleSize:
      {
        status = FILE_SOURCE_DATA_OK;
        break;
      }
    case PARSER_ErrorEndOfFile:
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "End of the Trak");
        status = FILE_SOURCE_DATA_END;
        break;
      }
    case PARSER_ErrorInsufficientBufSize:
      {
        status = FILE_SOURCE_DATA_REALLOCATE_BUFFER;
        break;
      }
    case PARSER_ErrorDataUnderRun:
      {
        status = FILE_SOURCE_DATA_INSUFFICIENT;
        break;
      }
    case PARSER_ErrorReadFail:
      {
        status = FILE_SOURCE_DATA_ERROR;
        break;
      }
    case PARSER_ErrorCodecInfo:
      {
        status = FILE_SOURCE_DATA_CODEC_INFO;
        break;
      }
    case PARSER_ErrorDefault       :
    case PARSER_ErrorInvalidParam  :
    case PARSER_ErrorInvalidTrackID:
      {
        status = FILE_SOURCE_DATA_INVALID;
        break;
      }
    case PARSER_ErrorUnsupportedCodecType:
    case PARSER_ErrorUnknownCodecType    :
    case PARSER_ErrorNotImplemented      :
      {
        status = FILE_SOURCE_DATA_NOT_IMPLEMENTED;
        break;
      }
    case PARSER_ErrorDataFragment      :
      {
        status = FILE_SOURCE_DATA_FRAGMENT;
        break;
      }
    /* Other errors are not known, So mapping these to some known status */
    default:
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource Error: %d", retError);
        status = FILE_SOURCE_DATA_END;
      }
  }
}
/*! ======================================================================
   @brief    Provides Audio Media Sample(s) for the requested track

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] numSample number of samples to retrieve
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextAudioSample() is called.
========================================================================== */
FileSourceMediaStatus FileSourceHelper::GetNextAudioFrame(uint32 id, uint8 *buf,
                                                           uint32 *size, int *numSample)
{
  FileSourceMediaStatus status = FILE_SOURCE_DATA_ERROR;
  bool bGetFrameAtBoundary = false;
  uint32 ulFrameBytes = 0;
  FileSourceFileFormat fileFormat = FILE_SOURCE_UNKNOWN;

  if(buf && size && numSample && m_pAudioFileHandle)
  {
    FileSourceConfigItem audioconfigItem;
    (void)GetConfiguration(id,&audioconfigItem,FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
    if(audioconfigItem.nresult)
    {
      bGetFrameAtBoundary = true;
    }
    if (NULL == m_pAudioFileHandle)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "m_pAudioFileHandle is NULL");
      return FILE_SOURCE_DATA_ERROR;
    }
    uint8 ucAudioType = m_pAudioFileHandle->getTrackOTIType(id);
    if(GetFileFormat(fileFormat) == FILE_SOURCE_SUCCESS)
    {
      //Also check audio format here
      if( (bGetFrameAtBoundary) &&
          (((fileFormat == FILE_SOURCE_AVI) && ((ucAudioType == MP3_AUDIO) ||
            (ucAudioType == AAC_ADTS_AUDIO) || (ucAudioType==AC3_AUDIO))) ||
          ((fileFormat == FILE_SOURCE_MP2TS) && (ucAudioType == AC3_AUDIO)) ||
          ((fileFormat == FILE_SOURCE_WFD_MP2TS) && (ucAudioType == AC3_AUDIO)) ||
          ((fileFormat == FILE_SOURCE_DASH_MP2TS) && (ucAudioType == AC3_AUDIO))))
      {
        if(!m_pAudioDataBuffer)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "GetNextAudioFrame Audio buffer not allocated");
          return FILE_SOURCE_DATA_MALLOC_FAILED;
        }
        else
        {
          if(m_pAudioDataBuffer && (m_pAudioDataBuffer->nDataBufSize == 0))
          {
            m_pAudioDataBuffer->nDataBufSize = m_pAudioFileHandle->getTrackMaxBufferSizeDB(id) * 2;
            m_pAudioDataBuffer->pDataBuff = (uint8*)MM_Malloc(m_pAudioDataBuffer->nDataBufSize);
            if(!m_pAudioDataBuffer->pDataBuff)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                           "GetNextAudioFrame FILE_SOURCE_DATA_MALLOC_FAILED");
              return FILE_SOURCE_DATA_MALLOC_FAILED;
            }
          }
        }

        while(1)
        {
          //If we have more than 4 bytes of data available
          if(m_pAudioDataBuffer->nDataSize - m_pAudioDataBuffer->nReadIndex > MIN_FRAME_HEADER_SIZE)
          {
            //Data already available, lets look for audio frames
            ulFrameBytes = LocateAudioFrameBoundary(id,m_pAudioDataBuffer,true);
            status = FILE_SOURCE_DATA_OK;
            if(!ulFrameBytes)
            {
              m_pAudioDataBuffer->nDataSize  = 0;
              m_pAudioDataBuffer->nReadIndex = 0;
            }
          }
          //If we have less than 4 bytes or do not have a complete frame
          if( (MIN_FRAME_HEADER_SIZE >= (m_pAudioDataBuffer->nDataSize - m_pAudioDataBuffer->nReadIndex)) ||
                (ulFrameBytes > (m_pAudioDataBuffer->nDataSize - m_pAudioDataBuffer->nReadIndex)))
          {
            //Enough data not available, lets get next sample
            *size = m_pAudioFileHandle->getTrackMaxBufferSizeDB(id);
            status = GetNextAudioSample(id,buf,size,numSample);
            if(status == FILE_SOURCE_DATA_OK)
            {
              //! Get the valid data start using ReadIndex parameter
              //! Calculate the remaining data available in buffer
              uint8* pucTemp =
                m_pAudioDataBuffer->pDataBuff + m_pAudioDataBuffer->nReadIndex;
              uint32 nRemainingData = m_pAudioDataBuffer->nDataSize -
                                      m_pAudioDataBuffer->nReadIndex;
              //Handle insufficient buffer size here
              if(*size + nRemainingData > m_pAudioDataBuffer->nDataBufSize)
              {
                uint8* ptemp = (uint8*)MM_Realloc(m_pAudioDataBuffer->pDataBuff,
                                                      m_pAudioDataBuffer->nDataBufSize * 2);
                if(!ptemp)
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSourceHelper::GetNextAudioFrame MM_Realloc failed");
                  return FILE_SOURCE_DATA_MALLOC_FAILED;
                }
                else
                {
                  m_pAudioDataBuffer->pDataBuff = ptemp;
                  m_pAudioDataBuffer->nDataBufSize = m_pAudioDataBuffer->nDataBufSize * 2;
                }
              }
              //! Retain remaining data available before copying the data
              memmove( m_pAudioDataBuffer->pDataBuff, pucTemp, nRemainingData);
              memcpy(m_pAudioDataBuffer->pDataBuff+nRemainingData,buf,*size);
              m_pAudioDataBuffer->nDataSize  = *size + nRemainingData;
              m_pAudioDataBuffer->nReadIndex = 0;
            }
            else
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "GetNextAudioFrame returning status %d",status);
              return status;
            }
          }
          //! If frame length is calculated and sufficient data is available
          //! break the loop
          if(ulFrameBytes &&
             (ulFrameBytes <= (m_pAudioDataBuffer->nDataSize - m_pAudioDataBuffer->nReadIndex)))
          {
            break;
          }
        }
        //! Copy frame size worth data from local cache to o/p buffer
        int nHdrSize = 0;
        uint8* pTemp = m_pAudioDataBuffer->pDataBuff+
                       m_pAudioDataBuffer->nReadIndex;
        *size = ulFrameBytes;

        /* We should strip header for AAC ADTS audio if config item is set. */
        if(ucAudioType == AAC_ADTS_AUDIO)
        {
          bool bStripAudioHeader = false;
          FileSourceConfigItem audioconfigItem;
          (void)GetConfiguration(id,&audioconfigItem,FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
          if(audioconfigItem.nresult)
          {
            bStripAudioHeader = true;
          }
          if((bStripAudioHeader) && (*size > AAC_ADTS_HEADER_SIZE) &&
             ((pTemp[0] == 0xFF) && ((pTemp[1] & 0xF0) == 0xF0)) )
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSourceHelper::GetNextAudioFrame \
              AAC_ADTS_AUDIO bStripAudioHeader");
            nHdrSize = AAC_ADTS_HEADER_SIZE;
          }
        } //! if(ucAudioType == AAC_ADTS_AUDIO)
        //! Copy data into o/p buffer and update ReadIndex to end of cur sample
        memcpy(buf,pTemp + nHdrSize,ulFrameBytes - nHdrSize);
        *size = ulFrameBytes - nHdrSize;
        m_pAudioDataBuffer->nReadIndex += ulFrameBytes;
      }
      else
      {
        status = GetNextAudioSample(id,buf,size,numSample);
      }
    }
    else
    {
      status = FILE_SOURCE_DATA_ERROR;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSourceHelper::GetNextAudioFrame GetFileFormat failed");
    }
  }
  else
  {
    status = FILE_SOURCE_DATA_ERROR;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSourceHelper::GetNextAudioFrame FILE_SOURCE_DATA_ERROR");
  }
  return status;
}
/*! ======================================================================
   @brief    Locates Frame boundary for audio

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] pSampleInfo Provides Information( eg: timestamp etc) about the sample
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextMediaSample() is called.
========================================================================== */
uint32 FileSourceHelper::LocateAudioFrameBoundary(uint32 ulTrackid,audio_data_buffer* pAudBuff,
                                                  bool bprocesstimestamp)
{
  uint32 ulFrameBytes = 0;
  bool bRet = false;
  bool bok = false;

  if(pAudBuff && pAudBuff->pDataBuff && m_pAudioFileHandle)
  {
    bRet = true;
    uint8* pucFramedata =  pAudBuff->pDataBuff + pAudBuff->nReadIndex;
    uint32 ulBufSize    = pAudBuff->nDataSize - pAudBuff->nReadIndex;
    uint8 ucAudioType   = m_pAudioFileHandle->getTrackOTIType(ulTrackid);

    if(ucAudioType == MP3_AUDIO)
    {
      for(uint32 i = 0; ulBufSize && (i < (ulBufSize-1)); i++)
      {
        if( (0xFF == pucFramedata[i]) && (0xE0 == (pucFramedata[i+1] & 0xE0)) )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Found MP3 sync word");
          bok = true;
        }
        if(bok)
        {
          tech_data_mp3 phdr;
          memset(&phdr,0,sizeof(tech_data_mp3));
          pAudBuff->nReadIndex+=i;

          ulFrameBytes = GetMP3AudioInfo(pucFramedata+i,&phdr,ulTrackid);
          if(ulFrameBytes)
          {
            if(bprocesstimestamp)
            {
              pAudBuff->nFrameTime = pAudBuff->nBaseTime + pAudBuff->nDelta;
              pAudBuff->nBaseTime += pAudBuff->nDelta;
              pAudBuff->nDelta = (MICROSEC_TIMESCALE_UNIT *
                        MP3_SAMPLES_TABLE[(phdr.version == MP3_VER_1 ? 0 : 1)]
                                         [phdr.layer] )
                        / (float)phdr.samplerate;
            }
            break;
          }
          else
          {
            pAudBuff->nReadIndex -= i;
          }
        }
        bok = false;
      }
    }
    else if(ucAudioType == AC3_AUDIO)
    {
      for(int i = 0; ulBufSize && (i < (int)(ulBufSize-1)) ; i++)
      {
        if((pucFramedata[i] == 0x0b)&&(pucFramedata[i+1] == 0x77))
        {
          uint32 ulBitrate = m_pAudioFileHandle->getTrackAverageBitrate(ulTrackid);
          uint32 ulFreq    = m_pAudioFileHandle->getTrackAudioSamplingFreq(ulTrackid);
          uint32 ulSamplingFreq = 0;
          /* bsid(offset = 5), 5-bit (11111xxx)*/
          uint8 ucBSID   = uint8(pucFramedata[i+5] >> 3);
          uint8 ucFscod  = (uint8)((pucFramedata[i+4]& 0xC0)>>6);
          ulSamplingFreq = AC3_FSCODE_RATE[ucFscod];
          pAudBuff->nReadIndex+=i;
            //! Frame size is stored in word format, convert to bytes
          if (IS_DOLBY_DIGITAL(ucBSID))
          {
            uint8 ucFrmsizecod = (uint8)(pucFramedata[i+4] & 0x3F);
            ulFrameBytes       = AC3_FRAME_SIZE_CODE
                                 [ucFrmsizecod].ulFrameSize[ucFscod];
            ulFrameBytes     <<= 1;
          }
          else if(IS_DOLBY_DIGITAL_PLUS(ucBSID))
          {
            //! Frame size does not include sync marker in EAC3 case, so
            //! include two bytes extra
            ulFrameBytes   = (pucFramedata[i+2] & 0x07) << 8 |
                             (pucFramedata[i+3]);
            //! Use half frequency parameter if sampling freq is ZERO
            if (0 == ulSamplingFreq)
            {
              uint8 ucFscod2 = (uint8)((pucFramedata[i+4]& 0x30)>>4);
              ulSamplingFreq= AC3_FSCODE_RATE[ucFscod2]/2;
            }
            ulFrameBytes<<= 1;
            ulFrameBytes += 2;
          }
          //! If Sampling frequency calculated is not matched with track
          //! property, then continue (Frame sync marker came as media data)
          if (ulSamplingFreq != ulFreq)
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "AC3 sync word emulation at %d, pb time %llu",
                         i, pAudBuff->nBaseTime);
            pAudBuff->nReadIndex -= i;
            continue;
          }
          //! For the first time, Update class structure
          else if (!sAC3HdrInfo.ucBSID)
          {
            sAC3HdrInfo.ucBSID          = ucBSID;
            sAC3HdrInfo.ulFrameLen      = ulFrameBytes;
            sAC3HdrInfo.ulSamplingFreq  = ulSamplingFreq;
          }
          //! If BSID is not matching with first sample value, then do not
          //! proceed, treat this as sync marker emulation
          else if (sAC3HdrInfo.ucBSID != ucBSID)
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "AC3 sync word emulation at %d, pb time %llu",
                         i, pAudBuff->nBaseTime);
            pAudBuff->nReadIndex -= i;
            continue;
          }

          pAudBuff->nFrameTime = (pAudBuff->nDelta+pAudBuff->nBaseTime);
          pAudBuff->nBaseTime += pAudBuff->nDelta;
          if(ulBitrate)
          {
            float nDelta = ulFrameBytes * 8;
            nDelta /= ulBitrate;
            nDelta *= MICROSEC_TIMESCALE_UNIT;
            pAudBuff->nDelta = nDelta;
          }
          break;
        }//if((framedata[i] == 0x0b)&&(m_pAudioDataBuffer->pDataBuff[i+1] == 0x77))
      }//for(uint32 i = 0; ndata && (i < (ndata-1)); i++)
    }
    else if(ucAudioType == AAC_ADTS_AUDIO)
    {
      for(int i = 0; ulBufSize && (i < (int)ulBufSize-1); i++)
      {
        uint16 uData = 0;
        uData = (uint16)((pucFramedata[i+1] << 8) + pucFramedata[i]);
        if (ADTS_HEADER_MASK_RESULT == (uData & ADTS_HEADER_MASK))
        {
          float frame_time = 0;
          ulFrameBytes = GetAACAudioInfo(pucFramedata+i,&frame_time);
          if(ulFrameBytes)
          {
            pAudBuff->nReadIndex+=i;
            pAudBuff->nFrameTime = (pAudBuff->nDelta + pAudBuff->nBaseTime);
            pAudBuff->nBaseTime += pAudBuff->nDelta;
            pAudBuff->nDelta = frame_time;
            break;
          }
        }
      }
    }
  }
  return ulFrameBytes;
}
/* ======================================================================
FUNCTION:
  FileSourceHelper::GetAACAudioInfo

DESCRIPTION:
  Parses the given buffer to find AAC ADTS header information.

INPUT/OUTPUT PARAMETERS:
  buf - buffer to parse for AAC info.
  frameTime - duration of the current frame obtained from the
              AAC ADTS frame header.

RETURN VALUE:
 uint32 - size of the current AAC ADTS frame

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FileSourceHelper::GetAACAudioInfo(uint8* pucBuf, float *pFrameTime)
{
  uint32 ulIndex = 0;
  bool bError = false;
  uint8 ucAudioObject = 0;
  uint8 ucChannelConfig = 0;
  uint64 ullFrameLength = 0;
  uint8 ucSamplingFreqIndex = 0;
  uint32 ulFrameBytes = 0;

  if(pFrameTime && pucBuf)
  {
    ucAudioObject = (uint8)( ((pucBuf[(ulIndex)+2] >> 6) & 0x03)+ 1);
    if(ucAudioObject > AAC_MAX_AUDIO_OBJECT)/*only 5 possible values*/
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAACAudioInfo audio_object > AAC_MAX_AUDIO_OBJECT");
      bError = true;
    }

    ucChannelConfig = (uint8)(((pucBuf [(ulIndex)+2] << 2) & 0x04)|
                              ((pucBuf [(ulIndex)+3] >> 6) & 0x03));
    if(ucChannelConfig > AAC_MAX_CHANNELS) /*only 48 possible values*/
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAACAudioInfo channelConfiguration > AAC_MAX_CHANNELS");
      bError = true;
    }
    ullFrameLength = (static_cast<uint64> (pucBuf[(ulIndex)+3] & 0x03) << 11)
                   | (static_cast<uint64> (pucBuf[(ulIndex)+4]) << 3)
                   | (static_cast<uint64> (pucBuf[(ulIndex)+5] & 0xE0) >> 5);

    if((ullFrameLength) && (!bError))
    {
      // parser framework handles max frame length of 32 bits
      ulFrameBytes = (uint32)(ullFrameLength & 0xFFFF);

      if(ulFrameBytes > 1500)
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,"GetAACAudioInfo frame_len is %lu, index is %lu",
                     ulFrameBytes, ulIndex);
        ullFrameLength = ulFrameBytes = 0;
      }

      ucSamplingFreqIndex = ((pucBuf[(ulIndex)+2] >> 2) & 0x0F);

      if(AAC_SAMPLING_FREQUENCY_TABLE[ucSamplingFreqIndex])
      {
        *pFrameTime = ((float)(AAC_SAMPLES_PER_DATA_BLOCK * MICROSEC_TIMESCALE_UNIT)/
                       AAC_SAMPLING_FREQUENCY_TABLE[ucSamplingFreqIndex]);
      }

      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetAACAudioInfo frameLength %lu, frame_time %llu",
        ulFrameBytes, (uint64)*pFrameTime);
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAACAudioInfo frameTime is NULL");
  }
  return ulFrameBytes;
}

/* ======================================================================
FUNCTION:
  FileSourceHelper::GetMP3AudioInfo

DESCRIPTION:
  Compares the present MP3 header with the next MP3 header to validate
  frame boundary logic.

INPUT/OUTPUT PARAMETERS:
  buf -  buffer to parse for MP3 info.
  phdr - pointer to MP3 header.
  trackid - id for the given track.

RETURN VALUE:
 uint32 - size of the current MP3 frame

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FileSourceHelper::GetMP3AudioInfo(uint8* pucBuf,void* phdr,uint32 ulTrackid)
{
  uint32 ulFrameBytes = 0;
  bool bRet = false;
  tech_data_mp3* pHdrInfo = (tech_data_mp3*)phdr;
  FileSourceMediaStatus retStatus = FILE_SOURCE_DATA_INVALID;

  bRet = FillMP3TechHdr(pucBuf,phdr,ulTrackid);
  if(bRet && pHdrInfo && pucBuf && m_pAudioFileHandle)
  {
    if (pHdrInfo->samplerate && pHdrInfo->bitrate && bRet)
    {
      if( (pHdrInfo->layer == MP3_LAYER_2)|| (pHdrInfo->layer == MP3_LAYER_3))
      {
        ulFrameBytes = (MP3_COEFFICIENTS[(pHdrInfo->version == MP3_VER_1 ? 0 : 1)][pHdrInfo->layer] * pHdrInfo->bitrate
                      / pHdrInfo->samplerate)* MP3_SLOT_SIZES[pHdrInfo->layer];
        if(pHdrInfo->is_padding)
        {
          ulFrameBytes++;
        }
      }
      else if(pHdrInfo->layer == MP3_LAYER_1)
      {
        ulFrameBytes = (12 * pHdrInfo->bitrate / pHdrInfo->samplerate + pHdrInfo->is_padding) * 4;
      }
    }
    /* To make frame boundary logic more robust, we will look into the immediate next MP3 header
       and look for sync word and other header fields and match it. */

    uint32 ulMaxSize = m_pAudioFileHandle->getTrackMaxBufferSizeDB(ulTrackid);
    uint32 ulSampleSize = ulMaxSize;
    /* sampleBuf allocated to hold the next mp3 frame needed for validation. */
    uint8* pucSampleBuf = (uint8*)MM_Malloc(ulSampleSize);
    bool bfoundSync = false;
    uint32 ulPosition = m_pAudioDataBuffer->nReadIndex;

    while(!bfoundSync)
    {
      /* If enough data is already read into the audioBuffer. */
      if((m_pAudioDataBuffer->nDataSize - ulPosition) > (ulFrameBytes + MIN_FRAME_HEADER_SIZE))
      {
        ulPosition += ulFrameBytes;
        uint8* framedata =  m_pAudioDataBuffer->pDataBuff + ulPosition;
        if( (0xFF == framedata[0]) && (0xE0 == (framedata[1] & 0xE0)) )
        {
          bfoundSync = true;
          tech_data_mp3 secondHdr;
          memset(&secondHdr, 0, sizeof(tech_data_mp3));
          bRet = FillMP3TechHdr(framedata,&secondHdr,ulTrackid);
          if( (bRet) &&
              (secondHdr.samplerate == pHdrInfo->samplerate) &&
              (secondHdr.channel == pHdrInfo->channel) &&
              (secondHdr.version == pHdrInfo->version) &&
              (secondHdr.layer == pHdrInfo->layer) )
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetMP3AudioInfo validated 2 frames successfully");
            break;
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetMP3AudioInfo validation failed");
            ulFrameBytes = 0;
            break;
          }
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetMP3AudioInfo validation failed, no sync word");
          ulFrameBytes = 0;
          break;
        }
      }
      else
      {
        /* Read more data into the audioBuffer. */
        int numSample;
        ulSampleSize = ulMaxSize;
        retStatus = GetNextAudioSample(ulTrackid,pucSampleBuf,&ulSampleSize,&numSample);
        if(retStatus == FILE_SOURCE_DATA_OK)
        {
          if((m_pAudioDataBuffer->nDataBufSize - m_pAudioDataBuffer->nDataSize) > ulSampleSize)
          {
            memcpy(m_pAudioDataBuffer->pDataBuff+m_pAudioDataBuffer->nDataSize,pucSampleBuf,ulSampleSize);
            m_pAudioDataBuffer->nDataSize += ulSampleSize;
          }
          else
          {
            uint8* ptemp = (uint8*)MM_Realloc(m_pAudioDataBuffer->pDataBuff,
                                                    m_pAudioDataBuffer->nDataBufSize * 2);
            if(!ptemp)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSourceHelper::GetNextAudioFrame MM_Realloc failed");
              return FILE_SOURCE_DATA_ERROR;
            }
            else
            {
              m_pAudioDataBuffer->pDataBuff = ptemp;
              m_pAudioDataBuffer->nDataBufSize = m_pAudioDataBuffer->nDataBufSize * 2;

              memcpy(m_pAudioDataBuffer->pDataBuff+m_pAudioDataBuffer->nDataSize,pucSampleBuf,ulSampleSize);
              m_pAudioDataBuffer->nDataSize += ulSampleSize;
            }
          }
        }
        else
        {
          ulFrameBytes = 0;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetMP3AudioInfo GetNextAudioSample retStatus %d",retStatus);
          break;
        }
      }
    }
    if( pucSampleBuf)
    {
      MM_Free(pucSampleBuf);
    }
  }
  return ulFrameBytes;
}
/* ======================================================================
FUNCTION:
  FileSourceHelper::FillMP3TechHdr

DESCRIPTION:
  Parses the given buffer to find MP3 header information.

INPUT/OUTPUT PARAMETERS:
  buf -  buffer to parse for MP3 info.
  phdr - pointer to MP3 header.
  trackid - id for the given track.

RETURN VALUE:
 uint32 - size of the current MP3 frame

SIDE EFFECTS:
  None.
======================================================================*/
bool FileSourceHelper::FillMP3TechHdr(uint8* buf,void* phdr,uint32 trackid)
{
  bool bRet = true;
  uint8 ucSampleindex = 0;
  int32 nBitrateFromParser = 0;

  if(m_pAudioFileHandle && phdr && buf)
  {
    uint32 ulSampleratefromparser = m_pAudioFileHandle->getTrackAudioSamplingFreq(trackid);
    uint8  ucChannelsfromparser   = (uint8)m_pAudioFileHandle->GetNumAudioChannels(trackid);
    tech_data_mp3* pHdrInfo = (tech_data_mp3*)phdr;

    //Do this only for VBR(need a api to check VBR for all parsers)
    nBitrateFromParser = m_pAudioFileHandle->getTrackAverageBitrate(trackid);

    pHdrInfo->version = (mp3_ver_enum_type)
   ((buf[MP3HDR_VERSION_OFS] & MP3HDR_VERSION_M) >> MP3HDR_VERSION_SHIFT);

    if (!(
          (MP3_VER_25 == pHdrInfo->version)||
          (MP3_VER_2 == pHdrInfo->version) ||
          (MP3_VER_1 == pHdrInfo->version)
         )
       )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr failed, invalid version..");
      bRet = false;
    }
    pHdrInfo->layer = (mp3_layer_enum_type)
        ((buf[MP3HDR_LAYER_OFS] & MP3HDR_LAYER_M) >> MP3HDR_LAYER_SHIFT);

    if (!(
           (MP3_LAYER_3 == pHdrInfo->layer)||
           (MP3_LAYER_2 == pHdrInfo->layer)||
           (MP3_LAYER_1 == pHdrInfo->layer)
         )
       )
    {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr failed,invalid layer number");
       bRet = false;
    }
    pHdrInfo->crc_present = (boolean)
      ((buf[MP3HDR_CRC_OFS] & MP3HDR_CRC_M) >> MP3HDR_CRC_SHIFT);

    byte bitrateIndex = ((buf[MP3HDR_BITRATE_OFS] & MP3HDR_BITRATE_M) >> MP3HDR_BITRATE_SHIFT);

    if (MP3_MAX_BITRATE_INDEX <= bitrateIndex)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr failed,invalid bit-rate");
      bRet = false;
    }

    pHdrInfo->max_bitrate = 1000 * MP3_BITRATE[(pHdrInfo->version == MP3_VER_1 ? 0 : 1)]
                           [pHdrInfo->layer][MP3_MAX_BITRATE_INDEX-1];
    pHdrInfo->bitrate = 1000 *
                MP3_BITRATE[(pHdrInfo->version == MP3_VER_1 ? 0 : 1)]
                [pHdrInfo->layer][bitrateIndex];

    ucSampleindex = (uint8)((buf[MP3HDR_SAMPLERATE_OFS] &
              MP3HDR_SAMPLERATE_M) >> MP3HDR_SAMPLERATE_SHIFT);

    if( MP3_MAX_SAMPLERATE_INDEX < ucSampleindex)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr failed,invalid sample rate index");
      bRet = false;
    }
    uint32 ulSamplefreq = MP3_SAMPLING_RATE[pHdrInfo->version][ucSampleindex];
    if((ulSamplefreq != ulSampleratefromparser)&& (ulSampleratefromparser))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr nsampleratefromparser != nsamplefreq");
      bRet = false;
    }

    pHdrInfo->samplerate = MP3_SAMPLING_RATE[pHdrInfo->version][ucSampleindex];
    pHdrInfo->is_padding = (boolean)((buf[MP3HDR_PADDING_OFS] & MP3HDR_PADDING_M) >> MP3HDR_PADDING_SHIFT);
    pHdrInfo->is_private = (boolean)
        ((buf[MP3HDR_PRIVATE_OFS] & MP3HDR_PRIVATE_M) >> MP3HDR_PRIVATE_SHIFT);
    pHdrInfo->channel = (mp3_channel_enum_type)
        ((buf[MP3HDR_CHANNEL_OFS] & MP3HDR_CHANNEL_M) >> MP3HDR_CHANNEL_SHIFT);

    uint8 nchannels = 0;

    switch(pHdrInfo->channel)
    {
      case MP3_CHANNEL_STEREO:
      case MP3_CHANNEL_JOINT_STEREO:
      case MP3_CHANNEL_DUAL:
      {
        nchannels = 2;
      }
      break;
      case MP3_CHANNEL_SINGLE:
      {
        nchannels = 1;
      }
      break;
      default:
      break;
    }
    if((nchannels != ucChannelsfromparser)&& (ucChannelsfromparser))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"FillMP3TechHdr nchannelsfromparser != nchannels");
      bRet = false;
    }

    pHdrInfo->extension = (mp3_ext_enum_type)
       ((buf[MP3HDR_CHANNEL_EXT_OFS] & MP3HDR_CHANNEL_EXT_M)
        >> MP3HDR_CHANNEL_EXT_SHIFT);
    pHdrInfo->copyright_present = (boolean)
       ((buf[MP3HDR_COPYRIGHT_OFS] & MP3HDR_COPYRIGHT_M)
        >> MP3HDR_COPYRIGHT_SHIFT);
    pHdrInfo->is_original = (boolean)
        ((buf[MP3HDR_ORIGINAL_OFS] & MP3HDR_ORIGINAL_M)
        >> MP3HDR_ORIGINAL_SHIFT);
    pHdrInfo->emphasis = (mp3_emphasis_enum_type)
        ((buf[MP3HDR_EMPHASIS_OFS] & MP3HDR_EMPHASIS_M)
          >> MP3HDR_EMPHASIS_SHIFT);
  }
  return bRet;
}
/*! ======================================================================
   @brief    Provides Audio Media Sample(s) for the requested track

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] numSample number of samples to retrieve
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextAudioSample() is called.
========================================================================== */
FileSourceMediaStatus FileSourceHelper::GetNextAudioSample(uint32 id, uint8 *buf,
                                                           uint32 *size, int *numSample)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  uint32 index;
  // NULL_AUDIO is a EVRC BLANK sample, it should play nothing (1 byte).
  //static const char NULL_AUDIO = 0x00;
  FileSourceMediaStatus status = FILE_SOURCE_DATA_INVALID;

  if (buf == NULL || size == NULL)
  {
    return status;
  }

  FileSourceMnMediaType minorType = GetFileSourceMinorType(id);

  if ( m_pAudioFileHandle )
  {
    switch ( minorType )
    {
    case FILE_SOURCE_MN_TYPE_EVRC:
      {
        uint8 evrcType = m_pAudioFileHandle->getTrackAudioFormat(id);
#ifdef FEATURE_FILESOURCE_3GP_PARSER
        if ( evrcType == (uint8)VIDEO_FMT_STREAM_AUDIO_EVRC_PV )
        {
          //Use buf+1 because the frame type is NOT prepended to the sample data.
          retError = m_pAudioFileHandle->getNextMediaSample(id,
                                                           (buf + EVRC_SAMPLE_HEADER_SIZE),
                                                            size, index);
          MapParserError2FileSourceStatus(retError, status);
        }
        else /* evrcType == VIDEO_FMT_STREAM_AUDIO_EVRC this is 3gpp type track */
#endif
        {
          /* Frame type is prepended to the sample data. */
          retError = m_pAudioFileHandle->getNextMediaSample(id, buf, size, index);
          MapParserError2FileSourceStatus(retError, status);
        }
      }
      break;

    case FILE_SOURCE_MN_TYPE_QCP:
    case FILE_SOURCE_MN_TYPE_QCELP:
    case FILE_SOURCE_MN_TYPE_EVRC_B:
    case FILE_SOURCE_MN_TYPE_EVRC_WB:
    case FILE_SOURCE_MN_TYPE_AAC:
    case FILE_SOURCE_MN_TYPE_AAC_ADTS:
    case FILE_SOURCE_MN_TYPE_AAC_ADIF:
    case FILE_SOURCE_MN_TYPE_AAC_LOAS:
    case FILE_SOURCE_MN_TYPE_MP3:
    case FILE_SOURCE_MN_TYPE_MP2:
    case FILE_SOURCE_MN_TYPE_DTS:
    case FILE_SOURCE_MN_TYPE_AC3:
    case FILE_SOURCE_MN_TYPE_EAC3:
    case FILE_SOURCE_MN_TYPE_PCM:
    case FILE_SOURCE_MN_TYPE_G711_ALAW:
    case FILE_SOURCE_MN_TYPE_G711_MULAW:
    case FILE_SOURCE_MN_TYPE_GSM_FR:
    case FILE_SOURCE_MN_TYPE_VORBIS:
    case FILE_SOURCE_MN_TYPE_FLAC:
    case FILE_SOURCE_MN_TYPE_OPUS:
    case FILE_SOURCE_MN_TYPE_BSAC:
    case FILE_SOURCE_MN_TYPE_AMR_WB:
    case FILE_SOURCE_MN_TYPE_AMR_WB_PLUS:
    case FILE_SOURCE_MN_TYPE_GSM_AMR:
      {
        retError = m_pAudioFileHandle->getNextMediaSample(id, buf, size, index);
        MapParserError2FileSourceStatus(retError, status);
      }
      break;

    case FILE_SOURCE_MN_TYPE_WMA:
    case FILE_SOURCE_MN_TYPE_WM_SPEECH:
    case FILE_SOURCE_MN_TYPE_WMA_PRO:
    case FILE_SOURCE_MN_TYPE_WMA_LOSSLESS:
      {
        retError = m_pAudioFileHandle->getNextMediaSample(id, buf, size, index);
        MapParserError2FileSourceStatus(retError, status);
      }
      if ( PARSER_ErrorNone == retError && *size)
      {
        if ( numSample )
        {
          file_sample_info_type SampleInfo;
          PARSER_ERRORTYPE returnCode = m_pAudioFileHandle->peekCurSample(id, &SampleInfo);
          if (returnCode == PARSER_ErrorNone)
          {
            *numSample = SampleInfo.num_frames;
          }
        }
        status = FILE_SOURCE_DATA_OK;
      }
      break;

    default:
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "Audio codec(%d) not supported in GetNextAudioSample",
                    minorType );
      break;
    }
  }

  return status;
}

/*! ======================================================================
   @brief    Provides Video Media Sample(s) for the requested track

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] numSample number of samples to retrieve
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextVideoSample() is called.
========================================================================== */
FileSourceMediaStatus FileSourceHelper::GetNextVideoSample(uint32 id, uint8* buf,
                                                           uint32* size, int* /*numSample*/)
{
  FileSourceMediaStatus status = FILE_SOURCE_DATA_INVALID;
  uint32 index =0;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;

  if (buf == NULL || size == NULL)
  {
    return status;
  }
  if ( m_pVideoFileHandle )
  {
    retError = m_pVideoFileHandle->getNextMediaSample(id, buf, size, index);
    MapParserError2FileSourceStatus(retError, status);
  }

  return status;
}

/*! ======================================================================
   @brief    Provides Text Media Sample(s) for requested track

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] numSample number of samples to retrieve
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextTextSample() is called.
========================================================================== */

FileSourceMediaStatus FileSourceHelper::GetNextTextSample(uint32 id, uint8* buf,
                                                          uint32* size, int* /*numSample*/)
{
  FileSourceMediaStatus status = FILE_SOURCE_DATA_INVALID;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  uint32 index;

  if (buf == NULL || size == NULL)
  {
    return status;
  }

  if ( m_pTextFileHandle )
  {
    retError = m_pTextFileHandle->getNextMediaSample(id, buf, size, index);
    MapParserError2FileSourceStatus(retError, status);
  }

  return status;
}

/*! ======================================================================
  @brief    Get information about the current sample of the track

  @param[in]      id            track ID
  @param[in/out]  pSampleInfo   Data structure with information about the sample
  @param[in]      pFileBase     File base pointer

  @return     none
  @note   It is expected that OpenFile() is called before
          GetCurrentSampleInfo() is called.
========================================================================== */

FileSourceMediaStatus FileSourceHelper::GetCurrentSampleInfo(uint32 id,
                                             FileSourceSampleInfo &pSampleInfo,
                                             FileBase* pFileBase)
{
  file_sample_info_type FileSampleInfo;
  uint64 StartTime = 0;
  uint64 EndTime = 0;
  uint64 delta = 0;
  FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
  PARSER_ERRORTYPE returnCode     = PARSER_ErrorNone;
  FileSourceMediaStatus status    = FILE_SOURCE_DATA_ERROR;
  FileSourceFileFormat fileFormat = FILE_SOURCE_UNKNOWN;
  bool bGetFrameAtBoundary = false;
  bool gotTS = false;

  memset(&FileSampleInfo, 0 , sizeof(file_sample_info_type));

  if ( pFileBase )
  {
    // Reset current value
    memset(&pSampleInfo, 0, sizeof(FileSourceSampleInfo));
    // peek the current sample
    returnCode = pFileBase->peekCurSample(id,&FileSampleInfo);

    // calculations to retrieve timing variables for a particular track
    if (returnCode == PARSER_ErrorNone)
    {
      FileSourceMjMediaType majorType;
      majorType = GetFileSourceMajorType(id);
      if(FILE_SOURCE_MJ_TYPE_AUDIO == majorType)
      {
        FileSourceConfigItem audioconfigItem;
        (void)GetConfiguration(id,&audioconfigItem,FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
        if(audioconfigItem.nresult)
        {
          bGetFrameAtBoundary = true;
        }
        uint8 audioType = m_pAudioFileHandle->getTrackOTIType(id);
        (void)GetFileFormat(fileFormat);
        if( (bGetFrameAtBoundary) &&
          (((fileFormat == FILE_SOURCE_AVI) && ((audioType == MP3_AUDIO) ||
                    (audioType == AC3_AUDIO) || (audioType == AAC_ADTS_AUDIO))) ||
          ((fileFormat == FILE_SOURCE_MP2TS) && (audioType == AC3_AUDIO)) ||
          ((fileFormat == FILE_SOURCE_WFD_MP2TS) && (audioType == AC3_AUDIO)) ||
          ((fileFormat == FILE_SOURCE_DASH_MP2TS) && (audioType == AC3_AUDIO))))
        {
          if(m_pAudioDataBuffer)
          {
            StartTime = (uint64)m_pAudioDataBuffer->nFrameTime;
            EndTime = (uint64)(m_pAudioDataBuffer->nFrameTime +
                               m_pAudioDataBuffer->nDelta);
            delta = (uint64)m_pAudioDataBuffer->nDelta;
            gotTS = true;
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSourceHelper::GetCurrentSampleInfo m_pAudioDataBuffer NULL");
          }
        }
      }
      if(!gotTS)
      {
        uint64 ulSampleTime = pFileBase->getMediaTimestampForCurrentSample(id);
        uint64 ulTimeScale  = pFileBase->getTrackMediaTimescale(id);

        delta = FileSampleInfo.delta;
        StartTime = ulSampleTime;
        if (ulTimeScale > 0 && ulTimeScale != MICROSEC_TIMESCALE_UNIT)
        {
          StartTime = (MICROSEC_TIMESCALE_UNIT * ulSampleTime) / ulTimeScale;
          delta = (MICROSEC_TIMESCALE_UNIT * delta) /ulTimeScale;
        }
        EndTime = StartTime + delta;
      }
      pSampleInfo.startTime = StartTime;
      pSampleInfo.endTime = EndTime;
      pSampleInfo.delta = delta;
      pSampleInfo.sync = FileSampleInfo.sync;
      pSampleInfo.bStartTsValid = FileSampleInfo.btimevalid;
      pSampleInfo.nBytesLost = FileSampleInfo.nBytesLost;
      pSampleInfo.nGranule   = FileSampleInfo.nGranule;
      pSampleInfo.nPCRValue  = FileSampleInfo.nPCRValue;

      //Update sample description index applicable to one set of samples
      pSampleInfo.sSubInfo.usSampleDescriptionIndex =
        FileSampleInfo.sSubInfo.usSampleDescriptionIndex;
      //Update content protection information if media is CP
      if( true == IsDRMProtection())
      {
        memcpy(&pSampleInfo.sSubInfo.sCP,
               &FileSampleInfo.sSubInfo.sCP,
               sizeof(FS_CONTENT_PROTECTION_INFOTYPE));
      }
      //Update subtitle track sub information
      if( ( FILE_SOURCE_SUCCESS == GetMimeType(id,majorType,minorType ) ) &&
          ( FILE_SOURCE_MJ_TYPE_TEXT == majorType ) )
      {
        memcpy(&pSampleInfo.sSubInfo.sSubTitle,
               &FileSampleInfo.sSubInfo.sSubTitle,
               sizeof(FS_SUBTITLE_SUB_INFOTYPE));
      }
      m_currentPosTimeStamp = StartTime;
    }//if(PARSER_ErrorNone)
  }//if( pFileBase )

  MapParserError2FileSourceStatus(returnCode, status);
  //print if return status is some error
  if(status != FILE_SOURCE_DATA_OK)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
               "GetCurrentSampleInfo for id %lu return status %d", id, status);
  }
  return status;
}

/*! ======================================================================
  @brief  local function to get index of the track in the MediaTrackInfo

  @param[in]  id    track ID
  @return     index
  @note   It is expected that OpenFile() is called before getIndexInMediaTrackInfo() is called.
========================================================================== */

int FileSourceHelper::GetIndexInMediaTrackInfo(uint32 id)
{

  int index = -1;
  uint32 i = 0;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "FileSource::getIndexInMediaTrackInfo id %lu", id);
  // search audioTrackInfo
  while ((index < 0)&&(i < m_nAudioTracks))
  {
    if ( id == m_audioTrackInfo[i].id)
    {
      index = i;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
       "FileSource::getIndexInMediaTrackInfo matched audio track id index %d",
      index);
    }
    i++;
  }

  // search videoTrackInfo
  i = 0;
  while ((index < 0)&&(i < m_nVideoTracks))
  {
    if ( id == m_videoTrackInfo[i].id)
    {
      index = i;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::getIndexInMediaTrackInfo matched video track id index %d",
      index);
    }
    i++;
  }

  // search textTrackInfo
  i = 0;
  while ((index < 0)&&(i < m_nTextTracks))
  {
    if ( id == m_textTrackInfo[i].id)
    {
      index = i;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
      "FileSource::getIndexInMediaTrackInfo matched text track id index %d",
      index);
    }
    i++;
  }
  return index;
}

/*! ======================================================================
  @brief  local function to get index of the track in the TrackInfo

  @param[in]  id    track ID
  @return     index
  @note   It is expected that OpenFile() is called before getIndexInTrackIdInfo() is called.
========================================================================== */

int FileSourceHelper::GetIndexInTrackIdInfo(uint32 id)
{
  int index = -1;
  uint32 i = 0;
  uint32 totalNumTracks = 0;
  MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_LOW,
  "FileSource::getIndexInTrackIdInfo id %lu m_nVideoTracks %lu m_nAudioTracks \
  %lu m_nTextTracks %lu",  id,m_nVideoTracks, m_nAudioTracks, m_nTextTracks);

  totalNumTracks = m_nVideoTracks + m_nAudioTracks + m_nTextTracks;

  // search for the track in the trackIdInfo
  while ((index < 0)&&(i < totalNumTracks))
  {
    if ( id == m_trackIdInfo[i].id)
    {
      index = i;
    }
    i++;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "FileSource::getIndexInTrackIdInfo index %d", index);
  return index;
}
/*! ======================================================================
//! destroys the media handles associated with the file being opened
 ======================================================================*/
void FileSourceHelper::DestroyMediaHandles()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::destroyMediaHandles");
  if(m_pAudioFileHandle)
  {
    MM_Delete(m_pAudioFileHandle);
  }
  if(m_pVideoFileHandle)
  {
    MM_Delete(m_pVideoFileHandle);
  }
  if(m_pTextFileHandle)
  {
    MM_Delete(m_pTextFileHandle);
  }
  m_pAudioFileHandle = NULL;
  m_pVideoFileHandle = NULL;
  m_pTextFileHandle = NULL;
}
/*! ======================================================================
    @brief  Enter the file open function.

    @detail The file source working thread entry function. Once the thread is created the control
            comes to this function.

    @param[in]  ptr  the "this" pointer.
    @return     status.

    @note   It is expected that when this function is called the file open thread has been
            created.
=========================================================================*/

int FileSourceHelper::SourceThreadEntry( void* ptr )
{
  FileSourceHelper* pThis = (FileSourceHelper *) ptr;

  if ( NULL != pThis )
  {
    pThis->SourceThread();
  }

  return 0;
}

/*! ======================================================================
    @brief  File source working  thread.

    @detail The file source working thread which handles commands and events posted to it.

    @param[in]  none.
    @return     none.
=========================================================================*/
void FileSourceHelper::SourceThread( void )
{
  bool bRunning = true;

  while ( bRunning )
  {
    /* Wait for a signal to be set on the signal Q. */
    uint32 *pEvent = NULL;

    if ( 0 == MM_SignalQ_Wait( m_pSignalQ, (void **) &pEvent ) )
    {
      switch ( *pEvent )
      {
        case OPEN_FILE_EVENT:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::SourceThread received OPEN_FILE_EVENT");
          /* Handle the file open command. */
          HandleOpenFileCommand();
          break;
        }
        case CLOSE_FILE_EVENT:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::SourceThread received CLOSE_FILE_EVENT");
          /* Handle the close file command. */
          HandleCloseFileCommand();
          break;
        }
        case SEEK_FILE_EVENT:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::SourceThread received SEEK_FILE_EVENT");
          /* Handle the seek file command. */
          HandleSeekFileCommand();
          break;
        }

        case THREAD_EXIT_EVENT:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::SourceThread received THREAD_EXIT_EVENT");
          /* Exit the thread. */
          bRunning = false;

#ifdef FEATURE_FILESOURCE_DRM_DCF
          ixipc_terminate();
#endif

          MM_Thread_Exit( m_pSourceThreadHandle, 0 );

          break;
        }

        default:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::SourceThread received UNKNOWN EVENT");
          /* Not a recognized event, ignore it. */
        }
      }
    }
  }
}
/*! ======================================================================
    @brief  Update media handle with downloaded data

    @detail In case of http streaming, each media handle needs to get an update
            notifying the maximum amount of data downloaded so far.
            This helps is determining if playback can begin.

    @param[in]  None.
    @return     None.

    @note   None.
=========================================================================*/
void FileSourceHelper::UpdateMediaBufferOffset(bool* bEndOfData)
{
  if(m_pIStreamPort)
  {
    *bEndOfData = false;
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    int64 nDownloadedBytes = 0;
    m_pIStreamPort->GetAvailableOffset(&nDownloadedBytes, bEndOfData);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::UpdateMediaBufferOffset nDownloadedBytes %lld",nDownloadedBytes);
      if(m_pAudioFileHandle)
      {
        m_pAudioFileHandle->updateBufferWritePtr(nDownloadedBytes);
      }
      if(m_pVideoFileHandle)
      {
        m_pVideoFileHandle->updateBufferWritePtr(nDownloadedBytes);
      }
      if(m_pTextFileHandle)
      {
        m_pTextFileHandle->updateBufferWritePtr(nDownloadedBytes);
      }
#endif
  }
}

/*! =======================================================================
    @brief      Create filebase instance from IStreamPort.

    @detail     Create audio file instance, once parsing is complete create
                other instances if the corresponding tracks exist.

    @param[in]  none.
    @return     none.

    @note       It is expected that when this function is called atleast
                one of the media handle is not NULL.
=========================================================================*/

void FileSourceHelper::OpenFileStreamPort(void)
{
#if defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) || defined (FEATURE_FILESOURCE_PSEUDO_STREAM)
  bool  bAudio,bVideo,bText;
  bool  bEndOfData            = false;
  bool  bAudioOnlyFormat      = IS_AUDIO_FILE_FORMAT(m_hFileFormatRequested);
  int64 nDownloadedBytes      = 0;

  bAudio = bText = false;
  bVideo = true;
  //! As per design, first video handle is created. But if file format is known
  //! as audio container such mp3/aac etc, there is no need to create video
  //! handle. This introduces extra delay in real streaming use case. Now Parser
  //! makes audio as true and video as false to create only one parser instance.
  if (true == bAudioOnlyFormat)
  {
    bAudio = true;
    bVideo = false;
  }

  /* Ensure both open and close flags are not set to true before creating
     new video handle */
  if ( (!m_bOpenPending) && (!m_bClosePending))
  {
    if( MM_CriticalSection_Create(&m_pCriticalSection) == 0 )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "m_pCriticalSection created..");
    }
    /* Following is the sequence of steps :
    1) Open Video Handle, Wait for Parsing to complete
    2) Once parsing is complete, check what types of tracks are present
    3) If Video track is not present, delete Video handle
    4) If Audio track is present, create audio handle
    5) If Text track is present, create text handle.
    */
    m_pVideoFileHandle = FileBase::openMediaFile(m_pIStreamPort,
                                                 bVideo, bAudio, bText,
                                                 m_bLookForCodecHdr,
                                                 m_hFileFormatRequested);
    if(m_pCriticalSection && m_pVideoFileHandle)
    {
      m_pVideoFileHandle->SetCriticalSection(m_pCriticalSection);
    }
  } // if ( (!m_bOpenPending) && (!m_bClosePending))
  else
  {
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                "m_bOpenPending %d, m_bClosePending %d, m_pVideoFileHandle %p",
                m_bOpenPending, m_bClosePending, m_pVideoFileHandle);
  }

  //! Update downloaded data offset value in Video handle
  if ((m_pVideoFileHandle) && (m_pIStreamPort))
  {
    m_pIStreamPort->GetAvailableOffset(&nDownloadedBytes, &bEndOfData);
    m_pVideoFileHandle->updateBufferWritePtr(nDownloadedBytes);
  }
  //! Before checking video handle status, check if Close is called or not
  //! If close is called, abort the video handle creation and report open fail
  if( (!m_bClosePending) && (m_pVideoFileHandle) && (m_pIStreamPort) &&
      (m_pVideoFileHandle->parseHTTPStream()) )
  {
    FileSourceFileFormat eFormat = m_hFileFormatRequested;
    if (FILE_SOURCE_UNKNOWN == eFormat)
    {
      (void)m_pVideoFileHandle->GetFileFormat(eFormat);
    }
    // Parsing successful, now create whatever handles.
    IdentifyTracksMajorType(m_pVideoFileHandle,&bAudio,&bVideo,&bText);
    if (!bVideo)
    {
      //! If file format is audio only, then use video file handle as audio
      //! file handle. This is to avoid the creation of same handle two times.
      if ((true == bAudioOnlyFormat) && (true == bAudio) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "FS::OpenFileStreamPort Video handle is used as Audio handle");
        m_pAudioFileHandle = m_pVideoFileHandle;
        m_pVideoFileHandle = NULL;
      }
      //! If audio instance is not required, then only destory video instance
      //! In case of MP2 Parser, Parser will get duration from video and set it
      //! on audio handle to reduce processing. Delete the handle after reading.
      else if(false == bAudio)
      {
        MM_Delete(m_pVideoFileHandle);
        m_pVideoFileHandle = NULL;
      }
    }
    else
    {
      FillVideoTrackInfo();
    }
    //! Check whether audio handle is already created or not
    if ((bAudio) && (!m_bClosePending) && (!m_pAudioFileHandle))
    {
      m_pAudioFileHandle = FileBase::openMediaFile(m_pIStreamPort,
                                                   false, true, false,
                                                   m_bLookForCodecHdr,
                                                   eFormat);
      if(m_pCriticalSection && m_pAudioFileHandle)
      {
        FS_LAST_PTS_VALUE sLastPTS;
        sLastPTS.ullLastPTS = 0;
        sLastPTS.eMajorType = FILE_SOURCE_MJ_TYPE_VIDEO;
        m_pVideoFileHandle->GetStreamParameter(0 /*Not required*/,
                                               FS_IndexParamLastPTSValue,
                                               &sLastPTS);
        m_pAudioFileHandle->SetStreamParameter(0 /*Not required*/,
                                               FS_IndexParamLastPTSValue,
                                               &sLastPTS);

        sLastPTS.eMajorType = FILE_SOURCE_MJ_TYPE_AUDIO;
        m_pVideoFileHandle->GetStreamParameter(0 /*Not required*/,
                                               FS_IndexParamLastPTSValue,
                                               &sLastPTS);
        m_pAudioFileHandle->SetStreamParameter(0 /*Not required*/,
                                               FS_IndexParamLastPTSValue,
                                               &sLastPTS);
        //! If video handle is not required, then delete the handle
        if(!bVideo)
        {
          MM_Delete(m_pVideoFileHandle);
          m_pVideoFileHandle = NULL;
        }

        m_pAudioFileHandle->SetCriticalSection(m_pCriticalSection);
      }
    } // if ((bAudio) && (!m_bClosePending))
    if ((bText) && (!m_bClosePending))
    {
      m_pTextFileHandle  = FileBase::openMediaFile(m_pIStreamPort,
                                                   false, false, true,
                                                   m_bLookForCodecHdr,
                                                   eFormat);
      if(m_pCriticalSection && m_pTextFileHandle)
      {
        m_pTextFileHandle->SetCriticalSection(m_pCriticalSection);
      }
    } // if ((bText) && (!m_bClosePending))
    //Update media handles about the amount of data downloaded.
    UpdateMediaBufferOffset(&bEndOfData);
    if (bAudio && m_pAudioFileHandle)
    {
      (void) m_pAudioFileHandle->parseHTTPStream();
      FillAudioTrackInfo();
    }
    if (bText && m_pTextFileHandle)
    {
      (void) m_pTextFileHandle->parseHTTPStream();
      FillTextTrackInfo();
    }
    m_bOpenPending = false;
  } // if( (!m_bClosePending) && ...)
  //! Do not continue, if Close request came
  else if((m_pVideoFileHandle) && (!m_bClosePending) )
  {
    PARSER_ERRORTYPE eRet = m_pVideoFileHandle->GetFileError();
    //! Reset open pending flag
    m_bOpenPending = false;

    /* Check whether close event is received or not. */
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
      "FS::OpenFileStreamPort m_bClosePending %d bEndOfData %d m_eState %d",
      m_bClosePending, bEndOfData, m_eState);
    if( (!bEndOfData) && (m_bFSAsync) &&
        (PARSER_ErrorInHeaderParsing != eRet))
    {
      MM_Timer_Sleep(20);
      /* Check whether close event is reached or not */
      if (!m_bClosePending)
      {
        m_bOpenPending = true;
        MM_Signal_Set( m_pOpenFileSignal );
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FileSource::HandleOpenFileCommand (int) m_bOpenPending %d",
                 m_bOpenPending);
    }
    else if (!bEndOfData)
    {
      m_bOpenPending = true;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "FileSource::HandleOpenFileCommand (int) m_bOpenPending %d",
                 m_bOpenPending);
    }
  } // else if(m_pVideoFileHandle)
  else
  {
    //! Reset open pending flag
    m_bOpenPending = false;
    if (!m_bClosePending)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "FileSource::HandleOpenFileCommand all media handles are \
                  NULL, not processing Open anymore..");
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "FileSource::HandleOpenFileCommand Close request came \
                  during Open, not processing Open anymore..");
    }
  }
  //! Reset FileSource state to init to execute OPEN next time if required
  m_eState = FS_STATE_INIT;

#endif/*(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) || defined (FEATURE_FILESOURCE_PSEUDO_STREAM)*/
}

/*! =======================================================================
    @brief      Create filebase instance from Buffer

    @detail     Create audio file instance, once parsing is complete create
                other instances if the corresponding tracks exist

    @param[in]  none
    @return     none

    @note       It is expected that when this function is called atleast
                one of the media handle is not NULL
=========================================================================*/
void FileSourceHelper::OpenFileBuffer(void)
  {
    bool bAudio,bVideo,bText;
    //By default, we will create one FileBase handle for each audio/video/text
    bAudio = bVideo = bText = true;

    if ( m_pAudioBuffer && m_nAudioBufSize )
    {
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
      m_pAudioFileHandle = FileBase::openMediaFile(m_pAudioBuffer, m_nAudioBufSize,false, true, false,m_bLookForCodecHdr,0,m_hFileFormatRequested);
#else
      m_pAudioFileHandle = FileBase::openMediaFile(m_pAudioBuffer, m_nAudioBufSize,false, true, false,m_hFileFormatRequested);
#endif
      if(m_pAudioFileHandle)
      {
        //Determine whether video/text exist before we create FileBase handle
        IdentifyTracksMajorType(m_pAudioFileHandle,&bAudio,&bVideo,&bText);
      }
      FillAudioTrackInfo();
    }
    if ( m_pVideoBuffer && m_nVideoBufSize && bVideo)
    {
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
      m_pVideoFileHandle = FileBase::openMediaFile(m_pVideoBuffer, m_nVideoBufSize,true, false, false,m_bLookForCodecHdr,0,m_hFileFormatRequested);
#else
      m_pVideoFileHandle = FileBase::openMediaFile(m_pVideoBuffer, m_nVideoBufSize,true, false, false,m_hFileFormatRequested);
#endif
      FillVideoTrackInfo();
    }
    if ( m_pTextBuffer && m_nTextBufSize && bText)
    {
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
      m_pTextFileHandle  = FileBase::openMediaFile(m_pTextBuffer,  m_nTextBufSize,false, false, true,m_bLookForCodecHdr,0,m_hFileFormatRequested);
#else
      m_pTextFileHandle  = FileBase::openMediaFile(m_pTextBuffer,  m_nTextBufSize,false, false, true,m_hFileFormatRequested);
#endif
      FillTextTrackInfo();
    }
}

/*! =======================================================================
    @brief      Create filebase instance from IxStream Handle

    @detail     Create audio file instance, once parsing is complete create
                other instances if the corresponding tracks exist.

    @param[in]  none.
    @return     none.

    @note       It is expected that when this function is called atleast
                one of the media handle is not NULL.
=========================================================================*/
void FileSourceHelper::OpenFileIxStream(void)
  {
#ifdef FEATURE_FILESOURCE_DRM_DCF
    bool bAudio,bVideo,bText;
    //By default, we will create one FileBase handle for each audio/video/text
    bAudio = bVideo = bText = true;
    if( MM_CriticalSection_Create(&m_pCriticalSection) == 0 )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "m_pCriticalSection created..");
    }
    m_pAudioFileHandle = FileBase::openMediaFile(m_pIxStream,false,true,false);
    if(m_pAudioFileHandle)
    {
      if(m_pCriticalSection)
      {
        m_pAudioFileHandle->SetCriticalSection(m_pCriticalSection);
      }
      //Determine whether video/text exist before we create FileBase handle
      IdentifyTracksMajorType(m_pAudioFileHandle,&bAudio,&bVideo,&bText);
      FillAudioTrackInfo();
    }
    if(bVideo)
    {
      m_pVideoFileHandle = FileBase::openMediaFile(m_pIxStream,true,false,false);
      if(m_pCriticalSection && m_pVideoFileHandle)
      {
        m_pVideoFileHandle->SetCriticalSection(m_pCriticalSection);
      }
      FillVideoTrackInfo();
    }
#endif
}

/*! =======================================================================
    @brief      Create filebase instance from File Name

    @detail     Create audio file instance, once parsing is complete create
                other instances if the corresponding tracks exist.

    @param[in]  none.
    @return     none.

    @note       It is expected that when this function is called atleast
                one of the media handle is not NULL.
=========================================================================*/
void FileSourceHelper::OpenLocalFile(void)
  {
    bool bAudio,bVideo,bText;
    bAudio = bVideo = bText = true;

    if ( m_audioFilename.get_size()  )
    {
      m_pAudioFileHandle = FileBase::openMediaFile(m_audioFilename,
                                                   false, true, false,
                                                   m_bLookForCodecHdr,
                                                   m_hFileFormatRequested);
      if(m_pAudioFileHandle)
      {
        //Determine whether video/text exist before we create FileBase handle
        IdentifyTracksMajorType(m_pAudioFileHandle,&bAudio,&bVideo,&bText);
      }
      FillAudioTrackInfo();
    }
    if ( m_videoFilename.get_size() && (bVideo) )
    {
      m_pVideoFileHandle = FileBase::openMediaFile(m_videoFilename,
                                                   true, false, false,
                                                   m_bLookForCodecHdr,
                                                   m_hFileFormatRequested);
      FillVideoTrackInfo();
    }
    if ( m_textFilename.get_size() && (bText) )
    {
      m_pTextFileHandle  = FileBase::openMediaFile(m_textFilename,
                                                   false, false, true,
                                                   m_bLookForCodecHdr,
                                                   m_hFileFormatRequested);
      FillTextTrackInfo();
    }
    ValidateMediaHandles();
  }

/*! ======================================================================
    @brief  Open local file.

    @detail Open the local file specified in the application thread.

    @param[in]  none.
    @return     none.

    @note   It is expected that when this function is called atleast one
            of the media handle is not NULL.
=========================================================================*/
void FileSourceHelper::HandleOpenFileCommand(void)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::HandleOpenFileCommand");

  //! Check status before issue Open command, it should be in INIT state
  if((FS_STATE_INIT == m_eState) || (FS_STATE_READY == m_eState) )
  {
    m_eState = FS_STATE_OPENING;
    if (m_pIStreamPort)
    {
      OpenFileStreamPort();
    }
    else if(m_pAudioBuffer || m_pVideoBuffer || m_pTextBuffer)
    {
      OpenFileBuffer();
    }
    else
#ifdef FEATURE_FILESOURCE_DRM_DCF
    if(m_pIxStream)
    {
      OpenFileIxStream();
    }
    else
#endif
    {
      OpenLocalFile();
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::OpenFile FS is not in valid state %d", m_eState);
  }

  CheckDRMError();
  //We may or may not have valid tracks, but change our state to READY as we are
  //done processing input stream/file
  m_eState = FS_STATE_READY;
  if( (!m_bOpenPending) && (m_nAudioTracks || m_nVideoTracks || m_nTextTracks))
  {
    if(m_nAudioTracks)
    {
      if( MM_CriticalSection_Create(&m_pAudioCriticalSection) == 0 )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "m_pAudioCriticalSection created..");
      }
    }
    if(m_nVideoTracks)
    {
      if( MM_CriticalSection_Create(&m_pVideoCriticalSection) == 0 )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "m_pVideoCriticalSection created..");
      }
    }
    if(m_nTextTracks)
    {
      if( MM_CriticalSection_Create(&m_pTextCriticalSection) == 0 )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "m_pTextCriticalSection created..");
      }
    }
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::HandleOpenFileCommand calling selectDefaultTracks");
    // Select default audio/video/text tracks
    SelectDefaultTracks();

    /*
     * For DRM playback, we only create DRM context in video instance.
     * Since we have separate media instance for audio/video/text,
     * need to setup DRM context in audio/text instances.
     */
    CopyDRMContextInfo();

    if (m_fileSourceHelperCallBackFunc)
    {
      if(m_pVideoFileHandle && m_pAudioFileHandle && m_pVideoFileHandle->isAviFileInstance())
      {
        (void)m_pAudioFileHandle->SetIDX1Cache(m_pVideoFileHandle->GetIDX1Cache());
      }
      if(m_pVideoFileHandle && m_pAudioFileHandle && m_pVideoFileHandle->isM2TSFileInstance() &&
        m_pAudioFileHandle->isM2TSFileInstance() )
      {
        FileSourceConfigItem audioconfigItem;
        FileSourceConfigItem videoconfigItem;
        uint64 baseTime = 0;
        FileSourceStatus metadataStatus = FILE_SOURCE_FAIL;

        metadataStatus = GetConfiguration((uint32)m_videoSelectedTrackId, &videoconfigItem, FILE_SOURCE_MEDIA_BASETIME);
        metadataStatus = GetConfiguration((uint32)m_audioSelectedTrackId, &audioconfigItem, FILE_SOURCE_MEDIA_BASETIME);
        baseTime = FILESOURCE_MIN(audioconfigItem.nresult,videoconfigItem.nresult);
        //We will set the MIN of Audio and Video baseTime as the baseTime for both
        if(baseTime == audioconfigItem.nresult)
        {
          metadataStatus = SetConfiguration((uint32)m_videoSelectedTrackId, &audioconfigItem, FILE_SOURCE_MEDIA_BASETIME);
        }
        else if(baseTime == videoconfigItem.nresult)
        {
          metadataStatus = SetConfiguration((uint32)m_audioSelectedTrackId, &videoconfigItem, FILE_SOURCE_MEDIA_BASETIME);
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "FileSourceStatus %d",metadataStatus);
      }
#if 0
      //! This Seek to ZERO is sometimes causing Parser to struck in unknown state.
      //! This is not useful operation with respect to Parser, Disabled this for
      //! now. If there is any explicit need for this, can be enabled later.
      bool berror = false;
      if(m_pVideoFileHandle && (m_videoSelectedTrackId >=0))
      {
        m_pVideoFileHandle->resetPlayback(0,m_videoSelectedTrackId,true,&berror,0);
      }
      if(m_pAudioFileHandle && (m_audioSelectedTrackId >=0))
      {
        m_pAudioFileHandle->resetPlayback(0,m_audioSelectedTrackId,false,&berror,0);
      }
      if(m_pTextFileHandle && (m_textSelectedTrackId >=0))
      {
        m_pTextFileHandle->resetPlayback(0,m_textSelectedTrackId,false,&berror,0);
      }
#endif
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::HandleOpenFileCommand reporting OPEN_COMPLETE");
      FileSourceFileFormat format = FILE_SOURCE_UNKNOWN;
      (void)GetFileFormat(format);
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileFormat %d",format);
      // revert to the callBack function provided by the client user with SUCCESS status
      m_fileSourceHelperCallBackFunc(FILE_SOURCE_OPEN_COMPLETE, m_pClientData);
    }
  }
  else if(m_fileSourceHelperCallBackFunc)
  {
    //In case of raw, report open_complete as codec/track data will be pushed later before playback starts.
    if(m_hFileFormatRequested == FILE_SOURCE_RAW)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::HandleOpenFileCommand reporting OPEN_COMPLETE");
      FileSourceFileFormat format = FILE_SOURCE_UNKNOWN;
      (void)GetFileFormat(format);
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileFormat %d",format);
      // report to the callBack function provided by the client user with SUCCESS status
      m_fileSourceHelperCallBackFunc(FILE_SOURCE_OPEN_COMPLETE, m_pClientData);
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "FileSource::HandleOpenFileCommand reported OPEN_COMPLETE to Client");
    }
    //We may not have the sufficient data to
    //begin playback when playing from iStreamPort.
    //Do not report open fail if we have Open Pending.
    else if (!m_bOpenPending)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::HandleOpenFileCommand reporting OPEN_FAIL");
      // report to the callBack function provided by cluent user with FAIL status
      m_fileSourceHelperCallBackFunc(FILE_SOURCE_OPEN_FAIL, m_pClientData);
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
            "FileSource::HandleOpenFileCommand reported OPEN_FAIL to Client");
    }
    else if (false == m_bFSAsync)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FileSource::HandleOpenFileCommand reporting OPEN_UNDERRUN");
      // report to the callBack function provided by cluent user with FAIL status
      m_fileSourceHelperCallBackFunc(FILE_SOURCE_OPEN_DATA_UNDERRUN, m_pClientData);
    }
  }
}

/*! ======================================================================
    @brief  Dump the text track to file

    @detail If text track exists, store the samples in file, works for
            WIN32 only

    @param[in]  none.
    @return     none.

    @note   It is expected that when this function is called atleast one
            of the media handle is not NULL.
=========================================================================*/

void FileSourceHelper::DumpTextTrack()
{
#ifdef PLATFORM_LTK
  if(m_nTextTracks && m_audioFilename.get_size() )
  {
    int namelength =  m_audioFilename.get_size()+1;
    //in case sourec file has extension less than 3 chars, we should make
    //the room for .srt in the worst case, we will end up allocating 4 bytes
    //more than what's neededbut it will make code simple/less ugly.
    namelength += strlen(".srt");
    char* pName= (char*)MM_Malloc(namelength);
    if(pName)
    {
      WideCharToChar(m_audioFilename.get_cstr(),std_wstrlen((AECHAR*)m_audioFilename.get_cstr()),pName,namelength);
      char* lastdot = std_strrchr(pName,'.');
      if(lastdot)
      {
        std_strlcpy(pName+(lastdot-pName),".srt",namelength);
        FILE* fp = fopen(pName,"wb");
        uint32 nbuf = GetTrackMaxFrameBufferSize(m_textTrackInfo[0].id);
        uint32 nmaxbufsize = nbuf;
        uint8* pbuf = (uint8*)malloc(nbuf);
        bool bok = true;
        FileSourceSampleInfo textInfo;
        int ncount = 1;
        while(bok && pbuf && fp)
        {
          if(pbuf)
          {
            nbuf = nmaxbufsize;
            if( FILE_SOURCE_DATA_OK != GetNextMediaSample(m_textTrackInfo[0].id,pbuf,&nbuf,textInfo))
            {
              bok = false;
            }
            else
            {
              //count
              if(ncount == 1)
              {
                fprintf(fp,"%d\n",ncount);
              }
              else
              {
                fprintf(fp,"\n\n%d\n",ncount);
              }
              uint64 nsec = textInfo.startTime/1000;
              uint64 nmsec = textInfo.startTime % 1000;
              uint64 nminutes = nsec/60;
              fprintf(fp,"00:%d:%d,%d -->",nminutes,nsec,nmsec);
              nsec = textInfo.endTime/1000;
              nmsec = textInfo.endTime % 1000;
              nminutes = nsec/60;
              fprintf(fp,"00:%d:%d,%d\n",nminutes,nsec,nmsec);
              //data
              //first 2 bytes has the size information
              fwrite(pbuf+2,1,(nbuf-2),fp);
              ncount++;
            }
          }//if(pbuf)
        }//while(bok && pbuf)
        if(fp)
        {
          fclose(fp);
        }
        if(pbuf)
        {
          free(pbuf);
        }
        if(pName)
        {
          free(pName);
        }
      }//if(lastdot)
    }//if(pName)
  }//if(m_nTextTracks && m_audioFilename.get_size() )
#endif //PLATFORM_LTK
}

/*! ======================================================================
//! Iterate through all of the existing tracks using given FileBase handle
//! to determine if audio/video/text track exist
========================================================================== */
void FileSourceHelper::IdentifyTracksMajorType(FileBase* pBase,
                                         bool* pHasAudio,
                                         bool* pHasVideo,
                                         bool* pHasText)
{
  if(pHasAudio)
  {
    *pHasAudio = false;
  }
  if(pHasVideo)
  {
    *pHasVideo = false;
  }
  if(pHasText)
  {
    *pHasText = false;
  }
  if(pBase)
  {
    uint32 numTracks = pBase->getNumTracks();
    uint32* idList = NULL;
    if ( numTracks > FILE_SOURCE_MAX_NUM_TRACKS )
    {
      //even though there are more tracks than what filesource supports,
      //we should allocate memory for all tracks otherwise
      //getTrackWholeIDList will cause buffer overflow in respective parser while filling in track id.
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Clip has more tracks than what FileSource supports!!!");
    }
    if (numTracks)
    {
      idList = MM_New_Array(uint32,(numTracks));
      if (idList)
      {
        (void)pBase->getTrackWholeIDList(idList);
      }
      //We should take MIN between number of tracks found in file, max. tracks supported in FileSource
      for (uint32 itr = 0; (itr < FILESOURCE_MIN(numTracks,FILE_SOURCE_MAX_NUM_TRACKS)) && (idList); itr++)
      {
        uint32 trackId = idList[itr];
        uint8 codecType = pBase->getTrackOTIType(trackId);
        if(IS_AUDIO_CODEC(codecType) && (pHasAudio) )
        {
          *pHasAudio = true;
        }
        if(IS_VIDEO_CODEC(codecType) && (pHasVideo) )
        {
          *pHasVideo = true;
        }
        if(IS_TEXT_CODEC(codecType) && (pHasText) )
        {
          *pHasText = true;
        }
      }
      if(idList)
      {
        MM_Delete_Array(idList);
      }
    }//if (numTracks)
  }//if(pBase)
}

/*! ======================================================================
//! close a file
========================================================================== */
void FileSourceHelper::HandleCloseFileCommand()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FileSource::HandleCloseFileCommand current state %d",m_eState);
  do
  {
    //! Check status before issue Close command
    if((FS_STATE_READY == m_eState) || (FS_STATE_INIT == m_eState))
    {
      break;
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                 "FileSource::HandleCloseFileCommand is not in READY state %d",
                 m_eState);
    MM_Timer_Sleep(20);
  } while (1);
  m_eState = FS_STATE_CLOSING;
  MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_MEDIUM,"FileSource::HandleCloseFileCommand");
  DestroyMediaHandles();
  BaseInitData();
  m_bClosePending = false;
  if (m_bOpenPending)
  {
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_MEDIUM,
              "FileSource::HandleCloseFileCommand, Open flag is set to true");
    m_bOpenPending = false;
  }
  //! Move Parser state to initialization after deleting all media handles
  m_eState = FS_STATE_INIT;
  if (m_bFSAsync)
  {
    MM_Signal_Set( m_pCloseFileDoneSignal );
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "FileSource::HandleCloseFileCommand CLOSE_COMPLETE");
}

/* ============================================================================
  @brief      Function to seek based on timestamp.

  @details    This function is used to do Seek operation on timestamp value.
              For Video track, Seek will be done to Sync sample if requested.
              For non-Video tracks, Seek will be done to non-Sync Samples.

  @param[in]      pFileBase         FileBase Handle.
  @param[in]      ulSelectedTrackId Track ID selected.
  @param[in]      slTrackIDtoSeek   Track ID on which Seek is requested.
  @param[in/out]  pbError           Error Flag Pointer.
  @param[in]      bSeektoSync       Flag to indicate whether sync sample based seek
                                    requested or not.
  @param[in/out]  ullReposTime      Reposition timestamp.
  @param[in]      pCriticalSection  Critical Section for handle.

  @return     Parser_ErrorNone if Seek is successful,
              else corresponding error.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE FileSourceHelper::SeekToTime(FileBase* pFileBase,
                                 int ulSelectedTrackId, int64 slTrackIDtoSeek,
                                 bool* pbError, bool bSeektoSync,
                                 uint64& ullReposTime, void* pCriticalSection)
{
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorInvalidParam;
  uint64 ullCurPosTime = m_nPlaybackTime;
  //! Validate input pointers
  if ((!pFileBase) || (!pCriticalSection) || (!pbError))
  {
    return eRetStatus;
  }

  *pbError   = false;
  eRetStatus = PARSER_ErrorNone;
  // Time to Seek
  ullReposTime  = m_nSeekAbsoluteTime;
  ullCurPosTime = m_currentPosTimeStamp;
  if(m_nPlaybackTime != -1)
  {
    ullCurPosTime = (uint64)m_nPlaybackTime;
  }

  if(FILE_SOURCE_SEEK_RELATIVE == m_eSeekType)
  {
    ullCurPosTime = m_nCurrentPlaybacktimeForSeek;
  }
  if ((-1 == slTrackIDtoSeek) || ((int)slTrackIDtoSeek == ulSelectedTrackId))
  {
    MM_CriticalSection_Enter(pCriticalSection);
    ullReposTime = pFileBase->resetPlayback(ullReposTime, ulSelectedTrackId,
                                          bSeektoSync, pbError, ullCurPosTime);
    MM_CriticalSection_Leave(pCriticalSection);
  }
  else
  {
    return PARSER_ErrorInvalidParam;
  }
  //! If it is video handle, try to do rewind operation in case of LA
  if((pbError) && (true == *pbError) && (m_pVideoFileHandle == pFileBase))
  {
#ifdef _ANDROID_
    FileSourceFileFormat eFileFormat;
    m_pVideoFileHandle->GetFileFormat(eFileFormat);
    if((FILE_SOURCE_ASF == eFileFormat ) &&
       (m_nSeekAbsoluteTime > m_currentPosTimeStamp) &&
       (m_nSeekAbsoluteTime > 10))
    {
      ullReposTime = m_nSeekAbsoluteTime;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                   "new rew seek time for ASF Clips is %llu",
                   ullReposTime -10);
      // rewind operation, if forward operation fails.
      ullReposTime = m_pVideoFileHandle->resetPlayback(ullReposTime -10,
                                                       m_videoSelectedTrackId,
                                                       m_bSeekToSync,
                                                       pbError, ullReposTime );
    }
    else if((FILE_SOURCE_3G2 == eFileFormat) ||
            (FILE_SOURCE_MPEG4 == eFileFormat))
    {
      /* If forward seek fails, Parser uses skipnsyncsamples API to goto
         previous sync frame. */
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                  "skipNSyncSamples called for 3g2 clips");
       ullReposTime = m_pVideoFileHandle->skipNSyncSamples(
                                            -1,
                                            m_videoSelectedTrackId,
                                            pbError,
                                            m_nCurrentPlaybacktimeForSeek);
    }
#endif
  }
  *pbError   = true;
  eRetStatus = pFileBase->GetFileError();
  if (PARSER_ErrorNone == eRetStatus)
  {
    *pbError = false;
    //! Update BaseTime only if the seek request came for audio track
    if ((-1 == slTrackIDtoSeek) || ((int)slTrackIDtoSeek == ulSelectedTrackId))
    if(m_pAudioFileHandle == pFileBase)
    {
      if(m_pAudioDataBuffer)
      {
        if(m_pAudioDataBuffer->pDataBuff)
        {
          MM_Free(m_pAudioDataBuffer->pDataBuff);
        }
        memset(m_pAudioDataBuffer,0,sizeof(audio_data_buffer));
        //! Basetime needs to be updated to seek time.
        //! Sample time is given in micro-sec units, whereas seek time is in
        //! milliseconds units. This parameter is used to calculate audio
        //! samples timestamp after seek.
        m_pAudioDataBuffer->nBaseTime = ullReposTime*1000;
      }
    }
  }
  return eRetStatus;
}

/* ============================================================================
  @brief      Function to seek based on timestamp.

  @details    This function is used to do Seek operation on timestamp value.
              For Video track, Seek will be done to Sync sample if requested.
              For non-Video tracks, Seek will be done to non-Sync Samples.

  @param[in]      ullReposTime  Reposition Seek time.

  @return     Parser_ErrorNone if Seek is successful,
              else corresponding error.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE FileSourceHelper::SeekAbsoluteAPI(uint64& ullReposTime)
{
  PARSER_ERRORTYPE retStatus = PARSER_ErrorSeekFail;
  bool   bError       = false;
  bool   bSeekDone    = false;
  uint64 ullSeekedTime= ullReposTime;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "FileSource::HandleSeekFileCommand Absolute Seek %llu ",
               m_nSeekAbsoluteTime);

  if(-1 != m_nTrackIdToSeek)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FileSource::HandleSeekFileCommand Seeking TrackId %lld ",
                 m_nTrackIdToSeek);
    //! If Track Id to seek is not any of default tracks, then update
    if( (m_audioSelectedTrackId != m_nTrackIdToSeek) &&
        (m_videoSelectedTrackId != m_nTrackIdToSeek) &&
        (m_textSelectedTrackId  != m_nTrackIdToSeek) )
    {
      (void)SetSelectedTrackID((uint32)m_nTrackIdToSeek);
    }
  }
  if (m_pVideoFileHandle)
  {
    retStatus = SeekToTime(m_pVideoFileHandle, m_videoSelectedTrackId,
                           m_nTrackIdToSeek, &bError, m_bSeekToSync,
                           ullReposTime, m_pVideoCriticalSection);
    if(PARSER_ErrorNone == retStatus)
    {
      ullSeekedTime = ullReposTime;
      bSeekDone = true;
    }
  }
  if (m_pAudioCriticalSection &&
      (m_audioSelectedTrackId >=0) &&
      m_pAudioFileHandle && !bError)
  {
    retStatus = SeekToTime(m_pAudioFileHandle, m_audioSelectedTrackId,
                           m_nTrackIdToSeek, &bError, false,
                           ullReposTime, m_pAudioCriticalSection);
    if(PARSER_ErrorNone == retStatus)
    {
      ullSeekedTime = ullReposTime;
      bSeekDone = true;
    }
  }
  if (m_pTextCriticalSection &&
      (m_textSelectedTrackId >=0) &&
      m_pTextFileHandle && !bError)
  {
    retStatus = SeekToTime(m_pTextFileHandle, m_textSelectedTrackId,
                           m_nTrackIdToSeek, &bError, false,
                           ullReposTime, m_pTextCriticalSection);
    if(PARSER_ErrorNone == retStatus)
    {
      ullSeekedTime = ullReposTime;
      bSeekDone = true;
    }
  }
  if(bSeekDone)
  {
    ullReposTime = ullSeekedTime;
    retStatus = PARSER_ErrorNone;
  }
  return retStatus;
}

/* ============================================================================
  @brief      Function to seek based on Sync Sample.

  @details    This function is used to do Seek operation on Sync Sample count.
              For Video track, Seek will be done to Sync sample if requested.
              For non-Video tracks, Seek will be done to non-Sync Samples.

  @param[in]      ullReposTime  Reposition Seek time.

  @return     Parser_ErrorNone if Seek is successful,
              else corresponding error.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE FileSourceHelper::SeekRelativeAPI(uint64& ullReposTime)
{
  PARSER_ERRORTYPE retStatus = PARSER_ErrorSeekFail;
  bool bError = false;
  // Time to Seek
  ullReposTime = m_nCurrentPlaybacktimeForSeek;
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
    "FileSource::HandleSeekFileCommand Relative Seek \
    m_nCurrentPlaybacktimeForSeek %llu m_nNumSync %d",
    m_nCurrentPlaybacktimeForSeek, m_nNumSync);

  if (m_pVideoFileHandle && (m_videoSelectedTrackId >=0) )
  {
    //Grab the critical section to make sure retrieval will not occur while seeking
    if(m_pVideoCriticalSection)
    {
      MM_CriticalSection_Enter(m_pVideoCriticalSection);
    }
    // reposition on the video track based on the number of the Sync samples
    ullReposTime = m_pVideoFileHandle->skipNSyncSamples(m_nNumSync,
                                                        m_videoSelectedTrackId,
                                                        &bError,
                                                        ullReposTime);
    retStatus = PARSER_ErrorNone;
    if(bError)
    {
      retStatus  = m_pVideoFileHandle->GetFileError();
    }
    if(m_pVideoCriticalSection)
    {
      MM_CriticalSection_Leave(m_pVideoCriticalSection);
    }
  } // if (m_pVideoFileHandle && (m_videoSelectedTrackId >=0) )

  if (m_pAudioFileHandle && (m_audioSelectedTrackId >=0) && (!bError))
  {
    retStatus = SeekToTime(m_pAudioFileHandle, m_audioSelectedTrackId,
                           m_audioSelectedTrackId, &bError, false,
                           ullReposTime, m_pAudioCriticalSection);
  } // if (m_pAudioFileHandle && (m_audioSelectedTrackId >=0) && (!bError))

  if (m_pTextFileHandle && (m_textSelectedTrackId >=0) && (!bError))
  {
    retStatus = SeekToTime(m_pTextFileHandle, m_textSelectedTrackId,
                           m_textSelectedTrackId, &bError, false,
                           ullReposTime, m_pTextCriticalSection);
  } //if (m_pTextFileHandle && (m_textSelectedTrackId >=0) && (!bError))
  return retStatus;
}

/* ============================================================================
  @brief      Function to return Seek status to FS client.

  @details    This function is used to return status through registered
              callback function.

  @param[in]      eRetStatus    Parser seek status.
  @param[in]      ullReposTime  Reposition Seek time.

  @return     Function will return Seek Callback status.

  @note       None.
============================================================================ */
void FileSourceHelper::ReturnSeekStatus(PARSER_ERRORTYPE eRetStatus,
                                        uint64 ullReposTime)
{
  FileSourceCallBackStatus eCBStatus = FILE_SOURCE_SEEK_FAIL;
  if (!m_fileSourceHelperCallBackFunc)
  {
    return;
  }
  switch(eRetStatus)
  {
    case PARSER_ErrorDataUnderRun:
      {
        eCBStatus = FILE_SOURCE_SEEK_UNDERRUN;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
         "HandleSeekFileCommand (FILE_SOURCE_SEEK_UNDERRUN) Seek time %llu",
         ullReposTime);
      }
      break;

    case PARSER_ErrorSeekUnderRunInFragment:
      {
        eCBStatus = FILE_SOURCE_SEEK_UNDERRUN_IN_FRAGMENT;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
          "HandleSeekFileCommand (FILE_SOURCE_SEEK_FRAGMENT) Seek time %llu",
          ullReposTime);
      }
      break;

    case PARSER_ErrorNone:
      {
        eCBStatus = FILE_SOURCE_SEEK_COMPLETE;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
          "HandleSeekFileCommand (FILE_SOURCE_SEEK_COMPLETE) Seek time %llu",
          ullReposTime);
      }
      break;
    default:
      {
        eCBStatus = FILE_SOURCE_SEEK_FAIL;
        // call the callBack function if FAIL.
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "FileSource::HandleSeekFileCommand FILE_SOURCE_SEEK_FAIL");
      }
      break;
  }
  //! Return Seek Callback status
  m_fileSourceHelperCallBackFunc(eCBStatus, m_pClientData);
  return;
}

/* ============================================================================
  @brief      Function to execute seek command.

  @details    This function is used to execute Seek command.

  @param[in]      None.

  @return     Function will return Seek Callback status.

  @note       None.
============================================================================ */
void FileSourceHelper::HandleSeekFileCommand()
{
  /* Please keep the callback function call to be the last operation of this
   * function. This will ensure no race condition.
   */
  PARSER_ERRORTYPE retStatus     = PARSER_ErrorSeekFail;
  uint64           ullReposTime  = 0;
  uint64           ullCurtPosTS  = 0;
  bool             bError        = false;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileSource::HandleSeekFileCommand");

  //! Check status before issue Seek command
  if(!IS_FILESOURCE_STATUS_OK(m_eState))
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FileSource::HandleSeekFileCommand is not in READY state %d",
                 m_eState);
    m_eSeekType = FILE_SOURCE_SEEK_UKNOWN;
  }

  if(FILE_SOURCE_SEEK_RELATIVE == m_eSeekType)
  {
    retStatus = SeekRelativeAPI(ullReposTime);
  } // if(FILE_SOURCE_SEEK_RELATIVE == m_eSeekType)
  else if (FILE_SOURCE_SEEK_ABSOLUTE == m_eSeekType)
  {
    retStatus = SeekAbsoluteAPI(ullReposTime);
  } // if(FILE_SOURCE_SEEK_ABSOLUTE == m_eSeekType)
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "FileSource::HandleSeekFileCommand unsupported SEEK TYPE");
  }

  //! Reset seek related variables
  m_eSeekType      = FILE_SOURCE_SEEK_UKNOWN;
  m_nPlaybackTime  = -1;
  m_nTrackIdToSeek = -1;
  m_bSeekToSync    = true;

  //! Return Seek Callback status
  ReturnSeekStatus(retStatus, ullReposTime);
}

/*! ======================================================================
  @brief  Deletes the respective media handle if audio/video/text does not exist
  @return none
========================================================================== */
void FileSourceHelper::ValidateMediaHandles()
{
  //don't delete media handle so that user can retrieve the specific error especially when DRM is enabled.
#ifndef FEATURE_FILESOURCE_DIVX_DRM
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::ValidateMediaHandles");
  //In case of RAW, HandleOpenFile will only result in creating RawFile handle.
  //FileSource user will push the codec data. Till then there won't be any recognizable tracks
  if( (m_pAudioFileHandle) && (!m_nAudioTracks) && (m_hFileFormatRequested != FILE_SOURCE_RAW) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Deleting audio media handle");
    MM_Delete(m_pAudioFileHandle);
    m_pAudioFileHandle = NULL;
  }
  if( (m_pVideoFileHandle) && (!m_nVideoTracks) && (m_hFileFormatRequested != FILE_SOURCE_RAW) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Deleting video media handle");
    MM_Delete(m_pVideoFileHandle);
    m_pVideoFileHandle = NULL;
  }
  if( (m_pTextFileHandle) && (!m_nTextTracks)&& (m_hFileFormatRequested != FILE_SOURCE_RAW) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Deleting text media handle");
    MM_Delete(m_pTextFileHandle);
    m_pTextFileHandle = NULL;
  }
#endif
}
//! checks if parser has encountered DRM related error.
//!If yes, cleans up the track information so that OPEN_FAIL will be reported.
void FileSourceHelper::CheckDRMError()
{
  bool berror = false;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"CheckDRMError");
  if(m_pAudioFileHandle)
  {
    int32 astatus = m_pAudioFileHandle->GetFileError();
    if((astatus == PARSER_ErrorDRMAuthorization)       ||
       (astatus == PARSER_ErrorDRMDeviceNotRegistered) ||
       (astatus == PARSER_ErrorDRMRentalCountExpired)  ||
       (astatus == PARSER_ErrorDRMMemAllocFail)        ||
       (astatus == PARSER_ErrorDRMPlaybackError))
    {
      berror = true;
    }
  }
  if(m_pVideoFileHandle)
  {
    int32 vstatus = m_pVideoFileHandle->GetFileError();
    if((vstatus == PARSER_ErrorDRMAuthorization)       ||
       (vstatus == PARSER_ErrorDRMDeviceNotRegistered) ||
       (vstatus == PARSER_ErrorDRMRentalCountExpired)  ||
       (vstatus == PARSER_ErrorDRMMemAllocFail)        ||
       (vstatus == PARSER_ErrorDRMPlaybackError))
    {
      berror = true;
    }
  }
#endif
  if(berror)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Detected DRM error..");
    for ( int i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
    {
      m_trackIdInfo[i].id = 0;
      m_trackIdInfo[i].selected = false;
      m_trackIdInfo[i].majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
    }
    for ( int i=0; i< FILE_SOURCE_MAX_NUM_TRACKS; i++)
    {
      (void) memset(&m_audioTrackInfo[i], 0, sizeof(FileSourceAudioInfo) );
      (void) memset(&m_videoTrackInfo[i], 0, sizeof(FileSourceVideoInfo) );
      (void) memset(&m_textTrackInfo[i],  0, sizeof(FileSourceTextInfo) );
    }
    m_nAudioTracks = 0;
    m_nVideoTracks = 0;
    m_nTextTracks = 0;
  }
}

/*! ======================================================================
  @brief  Copy DRM information from Videot o Text
  @return none
  @note   It is expected that CopyDRMContextInfo() is called from OpenFile()
========================================================================== */
FileSourceStatus FileSourceHelper::CopyDRMContextInfo()
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  /*
  * For DRM playback, we only create DRM context in video instance.
  * Since we have separate media instance for audio/video/text,
  * need to setup DRM context in audio/text instances.
  */
  if ( (m_pVideoFileHandle) &&(m_pVideoFileHandle->isAviFileInstance()) \
       && (m_pVideoFileHandle->IsDRMProtection())
     )
  {
    status = FILE_SOURCE_SUCCESS;
    if (m_pAudioFileHandle)
    {
      m_pAudioFileHandle->CopyDRMContextInfo((void*)m_pVideoFileHandle);
    }
    if (m_pTextFileHandle)
    {
      m_pTextFileHandle->CopyDRMContextInfo((void*)m_pVideoFileHandle);
    }
  }
#endif
  return status;
}

/*! ======================================================================
    @brief   Commit the DivX Playback.

    @param[in]    Major media type
    @return        true if commit is successful

    @note    It is expected that OpenFile() is called before CommitDivXPlayback() is called.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DIVX_DRM
bool FileSourceHelper::CommitDivXPlayback(FileSourceMjMediaType majorType)
#else
bool FileSourceHelper::CommitDivXPlayback(FileSourceMjMediaType)
#endif
{
  bool status = false;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  switch (majorType)
  {
  case FILE_SOURCE_MJ_TYPE_AUDIO:
    if (m_pAudioFileHandle)
    {
      status = m_pAudioFileHandle->CommitDivXPlayback();
    }break;
  case FILE_SOURCE_MJ_TYPE_VIDEO:
    if (m_pVideoFileHandle)
    {
      status = m_pVideoFileHandle->CommitDivXPlayback();
    }break;
  case FILE_SOURCE_MJ_TYPE_TEXT:
    if (m_pTextFileHandle)
    {
      status = m_pTextFileHandle->CommitDivXPlayback();
    }break;
  default: ;/* do nothing */
  }
#endif
  return status;
}

/*! ======================================================================
    @brief   Returns appropriate media handle.

    @param[in]    Track ID for which media handle is required
    @return        Pointer to media handle

    @note    It is expected that OpenFile() is called before GetMediaHandleForTrackID() is called.
========================================================================== */
FileBase* FileSourceHelper::GetMediaHandleForTrackID(uint32 id)
{
  FileBase* clipFileBase = NULL;
  for (uint32 i=0; i < FILE_SOURCE_MAX_NUM_TRACKS; i++)
  {
    if(m_trackIdInfo[i].id == id)
    {
      if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
      {
        clipFileBase = m_pAudioFileHandle;
        break;
      }
      else if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
      {
        clipFileBase = m_pVideoFileHandle;
        break;
      }
      else if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_TEXT)
      {
        clipFileBase = m_pTextFileHandle;
        break;
      }
    }
  }
  return clipFileBase;
}

/*! ===========================================================================
  @brief          Validated AAC file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateAACFile( uint8* pucDataBuffer,
                                         uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  bool isID3TagAvailable = false;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MIN_FILE_SIZE ) )
  {
    // if input buffer NULL, update pulBufferSize
    *pulBufferSize = MIN_FILE_SIZE;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else
  {
    uint32 ulID3TagLen = 0;
    uint32 ulSkipID3Len = 0;
    while(eRetStatus == FILE_SOURCE_FAIL)
    {
      if ( true == FileBase::IsID3TagPresent(pucDataBuffer, &ulID3TagLen))
      {
        isID3TagAvailable = true;
        ulSkipID3Len += ulID3TagLen;
        if(ulSkipID3Len + AAC_FORMAT_BUFF_SIZE > *pulBufferSize)
        {
          *pulBufferSize += ulID3TagLen + ID3V2_HEADER_SIZE+ AAC_FORMAT_BUFF_SIZE;
          eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
        }
        else
        {
          //Update Buffer pointer to start of frame sync marker
          pucDataBuffer+= ulID3TagLen + ID3V2_HEADER_SIZE;
        }
      }
      else
      {
        eRetStatus = FILE_SOURCE_SUCCESS;
        break;
      }
    }//while-loop to skip multiple ID3 tag.

    if(eRetStatus == FILE_SOURCE_SUCCESS)
    {
      PARSER_ERRORTYPE status = PARSER_ErrorDefault;

      //Update bufferSize value and Return status
      eRetStatus = FILE_SOURCE_FAIL;
      status =  FileBase::IsAACFormat(pucDataBuffer,
                                      *pulBufferSize - ulSkipID3Len);
      if (PARSER_ErrorNone == status)
      {
        eRetStatus = FILE_SOURCE_SUCCESS;
      }
      else if(isID3TagAvailable)
      {
        for(uint32 nCount = 0; nCount <  AAC_FORMAT_BUFF_SIZE; nCount++)
        {
          status = FileBase::IsAACFormat(pucDataBuffer + nCount,
                                         *pulBufferSize - nCount);
          if(PARSER_ErrorNone == status)
          {
            eRetStatus = FILE_SOURCE_SUCCESS;
            break;
          }
          else if((PARSER_ErrorDataUnderRun == status) ||
                   ((nCount + ulSkipID3Len) >= *pulBufferSize))
          {
            eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                        "ValidateAACFile data not sufficient");
            *pulBufferSize += AAC_FORMAT_BUFF_SIZE;
            break;
          }
        }
      }
      else
      {
        eRetStatus = FILE_SOURCE_FAIL;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "ValidateAACFile failed");
      }
    }
    else if(eRetStatus != FILE_SOURCE_DATA_NOTAVAILABLE)
    {
      eRetStatus = FILE_SOURCE_FAIL;
    }
  }//if (pucInputBuffer && *pulBufferSize)
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated MP3 file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateMP3File( uint8*  pucDataBuffer,
                                                   uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MIN_FILE_SIZE ) )
  {
    *pulBufferSize = MIN_FILE_SIZE;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else
  {
    uint32 ulID3TagLen = 0;
    uint32 ulSkipID3Len = 0;
    while(eRetStatus == FILE_SOURCE_FAIL)
    {
      if ( true == FileBase::IsID3TagPresent(pucDataBuffer, &ulID3TagLen))
      {
        ulSkipID3Len += ulID3TagLen;
        if(ulSkipID3Len > *pulBufferSize)
        {
          *pulBufferSize += ulID3TagLen + ID3V2_HEADER_SIZE;
          eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
        }
        else
        {
          //Update Buffer pointer to start of frame sync marker
          pucDataBuffer+= ulID3TagLen + ID3V2_HEADER_SIZE;
        }
      }
      else
      {
        eRetStatus = FILE_SOURCE_SUCCESS;
        break;
      }
    }//while-loop to skip multiple ID3 tag.
    if(eRetStatus == FILE_SOURCE_SUCCESS)
    {
      PARSER_ERRORTYPE status = FileBase::IsMP3Format(pucDataBuffer,
                                                *pulBufferSize - ulSkipID3Len);
      if (PARSER_ErrorNone == status)
      {
        eRetStatus = FILE_SOURCE_SUCCESS;
      }
      else if (PARSER_ErrorDataUnderRun == status)
      {
        eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
        *pulBufferSize += AAC_FORMAT_BUFF_SIZE;
      }
      else
      {
        eRetStatus = FILE_SOURCE_FAIL;
      }
    }
    else if(eRetStatus != FILE_SOURCE_DATA_NOTAVAILABLE)
    {
      eRetStatus = FILE_SOURCE_FAIL;
    }
  }//if (pucInputBuffer && *pulBufferSize)
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated AC3 file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateAC3File( uint8* pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < AC3_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = AC3_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsAC3File(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated DTS file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function
                  will updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is
                          not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not
                  have enough data. Then pBufferSize will be updated to have
                  required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateDTSFile( uint8* pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < DTS_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = DTS_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsDTSFile(pucDataBuffer, *pulBufferSize))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated AMR file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateAMRFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < AMR_FILE_SIGNATURE_BYTES ))
  {
    *pulBufferSize = AMR_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsAMRFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated AMR-WB file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateAMRWBFile( uint8*  pucDataBuffer,
                                                      uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < AMR_WB_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = AMR_WB_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsAMRWBFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated ASF file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateASFFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < ASF_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = ASF_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsASFFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated AVI file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateAVIFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_INVALID;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < AVI_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = AVI_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsAVIFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated EVRCB file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateEVRCBFile( uint8*  pucDataBuffer,
                                                     uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < EVRC_B_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = EVRC_B_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsEVRCBFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated EVRC-WB file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateEVRCWBFile( uint8*  pucDataBuffer,
                                                       uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < EVRC_WB_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = EVRC_WB_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsEVRCWBFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated FLAC file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateFLACFile( uint8*  pucDataBuffer,
                                                     uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < FLAC_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = FLAC_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsFlacFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated 3GP/MP4 file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateMP4File( uint8*  pucDataBuffer,
                                                     uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MP4_3GPP2_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = MP4_3GPP2_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( FileBase::IsMP4_3GPFile(NULL,pucDataBuffer, *pulBufferSize, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated 3GPP2 file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::Validate3GPP2File( uint8*  pucDataBuffer,
                                                      uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MP4_3GPP2_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = MP4_3GPP2_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  //! Continue further only if atom type is "FTYP"
  else if(!memcmp(pucDataBuffer+4, MP4_3GP_ATOM_FTYP,FOURCC_SIGNATURE_BYTES))
  {
    if( true == FileBase::Is3GPP2File(NULL,pucDataBuffer,*pulBufferSize,false ))
    {
      eRetStatus = FILE_SOURCE_SUCCESS;
    }
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated VOB file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateVOBFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  bool bIsProgStream;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MPEG2_PS_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = MPEG2_PS_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsMPEG2File(NULL,pucDataBuffer, *pulBufferSize,
                                         false, &bIsProgStream ))
  {
    if(bIsProgStream)
    {
      eRetStatus = FILE_SOURCE_SUCCESS;
    }
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated TS file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateTSFile( uint8*  pucDataBuffer,
                                                   uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  bool bIsProgStream=false;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < 4*MPEG2_M2TS_PKT_BYTES ) )
  {
    *pulBufferSize = 4*MPEG2_M2TS_PKT_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsMPEG2File(NULL,pucDataBuffer, *pulBufferSize,
                                         false, &bIsProgStream ))
  {
    if(!bIsProgStream)
    {
      eRetStatus = FILE_SOURCE_SUCCESS;
    }
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated MKV file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateMKVFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < MKV_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = MKV_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsMKVFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated FLV file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateFLVFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < FLV_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = FLV_SIGNATURE_BYTES;
    eRetStatus     = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsFLVFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return eRetStatus;
}

/*! ===========================================================================
  @brief          Validated OGG file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateOGGFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < OGG_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = OGG_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsOggFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated QCP file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateQCPFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < QCP_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = QCP_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsQCPFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

/*! ===========================================================================
  @brief          Validated WAV file format based on file format

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.
  @param[in/out]  pulBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
FileSourceStatus FileSourceHelper::ValidateWAVFile( uint8*  pucDataBuffer,
                                                    uint32* pulBufferSize )
{
  FileSourceStatus eRetStatus = FILE_SOURCE_FAIL;
  if ( ( NULL == pucDataBuffer ) ||
       ( *pulBufferSize < WAVADPCM_FILE_SIGNATURE_BYTES ) )
  {
    *pulBufferSize = WAVADPCM_FILE_SIGNATURE_BYTES;
    eRetStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
  }
  else if( true == FileBase::IsWAVADPCMFile(NULL,pucDataBuffer, false ))
  {
    eRetStatus = FILE_SOURCE_SUCCESS;
  }
  return (eRetStatus);
}

 /*!===========================================================================
  @brief      Get Audio/Video/Text stream parameter

  @details    This function is used to get Audio/Video stream parameter i.e.
              codec configuration, profile, level from specific parser.

  @param[in]  ulTrackId           TrackID of media
  @param[in]  ulParamIndex        Parameter Index of the structure to be
                                  filled.It is from the FS_MEDIA_INDEXTYPE
                                  enumeration.
  @param[in]  pParameterStructure Pointer to client allocated structure to
                                  be filled by the underlying parser.

  @return     PARSER_ErrorNone in case of success otherwise Error.
  @note
  ============================================================================*/
FileSourceStatus FileSourceHelper::GetStreamParameter( uint32 ulTrackId,
                                                      uint32 ulParamIndex,
                                                      void* pParamStruct)
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  FileBase* pMediaHandle = NULL;

  pMediaHandle = GetMediaHandleForTrackID(ulTrackId);
  if( ( NULL != pParamStruct ) &&
      ( NULL != pMediaHandle ) &&
      (PARSER_ErrorNone == pMediaHandle->GetStreamParameter(ulTrackId,
                                                            ulParamIndex,
                                                            pParamStruct ) ) )
  {
    eStatus = FILE_SOURCE_SUCCESS;
  }//if(pParamStruct && pMediaHandle )
  return eStatus;
}
/*! ======================================================================
    @brief   Returns appropriate Critical Section Pointer.

    @param[in]    Track ID for which Critcal Section Ptr is required
    @return       Pointer to media handle

    @note    It is expected that OpenFile() is called before
             GetCriticalSectionPtrForTrackID() is called.
========================================================================== */
void* FileSourceHelper::GetCriticalSectionPtrForTrackID(uint32 ulTrackID)
{
  void* pCritcalSectionPtr = NULL;
  for (uint32 i=0; i < FILE_SOURCE_MAX_NUM_TRACKS; i++)
  {
    if(m_trackIdInfo[i].id == ulTrackID)
    {
      if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
      {
        pCritcalSectionPtr = m_pAudioCriticalSection;
        break;
      }
      else if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
      {
        pCritcalSectionPtr = m_pVideoCriticalSection;
        break;
      }
      else if(m_trackIdInfo[i].majorType == FILE_SOURCE_MJ_TYPE_TEXT)
      {
        pCritcalSectionPtr = m_pTextCriticalSection;
        break;
      }
    }
  }
  return pCritcalSectionPtr;
}

