#ifndef __FILEMUX_H__
#define __FILEMUX_H__

/* =======================================================================
                              filemux.h
DESCRIPTION
  This is class for MUX.
  Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/FileMuxLib/inc/filemux.h#2 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "filemuxinternaldefs.h"
#include "isucceedfail.h"
#include "filemuxtypes.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "MMSignal.h"
#include "MMThread.h"
#include "muxbase.h"

class MUXBase;
class muxqueue;


typedef void * MM_HANDLE;

#if defined WINCE || defined PLATFORM_LTK
  #define FILEMUX_DLL __declspec( dllexport )
#else
  #define FILEMUX_DLL
#endif

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/*!
  @brief   FileMux Module.

  @detail The Intent of having this module is to Provide a common interface for
          upperlayers to control the MUX (for both MP4, AVI etc.. file formats)
		  and mux the audio/video/text/data streams into single stream and generate
		  necessary meta data and write into the application supplied
		  output port/flash/MMC/memory/client. It also manages the reordering
		  and temporary storage management if necessary.


  @note   Inorder to use the functions provided by this class. It is required
          that the User calls CreateMuxInstance before calling others
*/
class FILEMUX_DLL FileMux
{
private:
  uint8* m_nAudBuffer;
public:
/* Creates MUX instance */
/*!
   @brief   Creates a MUX instance

   @detail  It takes file_format, file_brand, output_handle, notification, callBack and pClientData as input parameters.
            Params will provide the necessary information to configure the writer library.
            file_format will tell the file format that we are going to record.
			file_brand will tell the Major brand that we are going to record.
			output_handle - The output that ultimately we are going to write.
            callBack - Notifaction call back to the application.
            MUX Module will get the audio/video/text/data frames/samples, UUID, telop etc.. fomats and writes into the o/p.

   @param[in]  Params will provide the necessary information to configure the writer library.
               file_format will tell the file format that we are going to record.
	           file_brand will tell the Major brand that we are going to record.
               output_handle - The output that ultimately we are going to write.
               callBack - Notifaction call back to the application.

   @return    Pointer to MUX base instance.

   @note    It is expected that except pOputStream nothing should be NULL.
   */
  FileMux(MUX_create_params_type *Params
               , MUX_fmt_type file_format
               , MUX_brand_type file_brand
               , MUX_handle_type *output_handle
               , boolean reorder_atom
               , MUXCallbackFuncType callBack
               , void *pClientData
		);


  //! MUXBase destructor gets called when delete the interface
  virtual ~FileMux();

#ifdef FILEMUX_WRITE_ASYNC
  //! callback function to receive data from FileMux
  static void callbackToWriteData(
                         void           *client_data  //! Client data from calling function
                         );
#endif  /* FILEMUX_WRITE_ASYNC  */


  /**************************************************************************
   ** Public Methods
   *************************************************************************/
public:

/*!
   @detail   Get Current MUX status

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_get_Status();

/*!
   @detail   Stop the MUX

   @detail   Done recording so stop the processing.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_end_Processing(void *pClientData);

/*!
   @detail   Write Meta Data

   @detail   This is used to set meta data to be written to file. .

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_set_MetaData(FileMuxMetaDataType eType,
                            FileMuxMetaData *);

/*!
   @brief   set statistics query  timer interval

   @detail  set statistics query  timer interval

    @return    MUX_STATUS.
*/

MUX_STATUS MUX_set_StatisticsInterval(uint64 interval);


/*!
   @brief   timer handler function

   @detail  timer handler function for statistics query

    @return    void.
*/

static void MUX_StatisticsInterval_TimerHandler(void *);

/*!
   @brief   Get the Recorded/Available space/time from ISO basefile

   @detail  Get the Recorded/Available space/time from ISO basefile

    @return    MUX_STATUS.
*/

MUX_STATUS MUX_get_Statistics();

/*!
    @brief  call to pause the MUX processing

    @detail call to pause the MUX processing

    @param[in]  pClientData context data

    @return     MUX_STATUS.
*/
MUX_STATUS MUX_pause_Processing(void *pClientData);



/*!
    @brief  call to resume the MUX processing

    @detail call to resume the MUX processing

    @param[in]  pClientData context data

    @return     MUX_STATUS.
*/
MUX_STATUS MUX_resume_Processing(void *pClientData);


/*!
   @brief   Processes the given audio/video/text/data sample

   @detail  It takes audio/video/text/data sample and processes.

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               num_samples - num of samples
			   sample_info - Sample info
			   sample_data - Actual sample data.
    @return    MUX_STATUS.

   @note    It is expected that except pClientData nothing should be NULL.
   */

MUX_STATUS MUX_Process_Sample( uint32 stream_number,
                                 uint32 num_samples,
                                 const MUX_sample_info_type  *sample_info,
                                 const uint8  *sample_data,
								 void *pClientData);
/*!
   @brief   Write the header to the movie file

   @detail  Write the header to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               audio_header_size - Length of the footer
			   audio_header - audio headter pointer
    @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_header (uint32 audio_str, uint32 audio_header_size, const uint8 *audio_header);

/*!
   @brief   Write the header to the movie file

   @detail  Write the header to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               bAsync        - To process synchronously or Asynchronously
               audio_header_size - Length of the footer
			   audio_header - audio headter pointer
    @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_header (uint32 audio_str,bool bAsync, uint32 audio_header_size,
                                    const uint8 *audio_header, void *pClientData);

/*!
   @brief   Write the footer to the movie file

   @detail  Write the footer to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               footer_length - Length of the footer
			   buf_ptr - Buffer pointer
    @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_footer (uint32 stream_num, uint32 footer_length, const uint8 *buf_ptr);
/*!
   @brief   Write the header text movie file

   @detail  Write the header text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               entry - Text to write
    @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_header_text (uint32 stream_num, const MUX_text_type_t *entry);
/*!
   @brief   Write the text sample to movie file

   @detail  Write the text text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               handle -
			   delta -
    @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_text (uint32 stream_num, void * handle, uint32 delta);

/*!
   @brief  To release a timed text stream sample without writing it to the stream.

   @detail  To release a timed text stream sample without writing it to the stream.

   @param[in]  handle - To Release

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_free_text (void * handle);
/*!
   @brief  This is used to pass user data atom information to be written to
           the header of the movie file.

   @detail  This is used to pass user data atom information to be written to
            the header of the movie file.

   @param[in] uuid - 16-byte UUID of the user atom
              data - Data
              size - Size

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_uuid (const uint8 *uuid, const void *data, uint32 size);
/*!
   @brief  This is used to update the UDTA atom with the given data

   @detail    This is used to update the UDTA atom with the given data

   @param[in] data - data to be updated with.
              size - size of the data.

   @return    MUX_STATUS.
*/

MUX_STATUS MUX_write_user_meta_data (const void *data, uint32 size);
/*!
   @brief   Mux given data in every timeout to the output handler

   @detail    Mux given data in every timeout to the output handler

   @param[in] data - data to be updated with.
              size - size of the data.
              timeout - the timeout of the write call back function in millseconds.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_write_user_meta_data (const void *data, uint32 size, uint32 timeout);

/*!
   @brief  This is used to update the mehd with total duration.

   @detail   This is used to update the mehd with total duration.

   @param[in] total_duration - total duration to be written.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_update_mehd_atom (uint32 total_duration);

/*!
   @brief  This is used to update the streamport.

   @detail   This is used to update the streamport.

   @param[in] port - new streamport instance.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_update_streamport(uint64_t port);

/*!
   @brief  This is used to update the clipinfo.

   @detail   This is used to update the clipinfo.

   @param[in] msg_list - list of messages with clip info.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_update_clipinfo (MUX_msg_list_type *msg_list);

/*!
   @brief  This is used to update AVC Timing and HRD params.

   @detail   Params used to populate the Timing and
             HRD descriptor in AVC is set using this function.

   @param[in] pHRDParams - pointer to Timing and HRD params structure.

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_update_AVC_Timing_HRD_Params(MUX_AVC_TimingHRD_params_type *pHRDParams);


/*!
   @brief  This is used to modify user data atoms.

   @detail   This is used to modify user data atoms.

   @param[in] puser_data - user data atom to be modified.
              handle   -  handle

   @return    MUX_STATUS.
*/
MUX_STATUS MUX_modify_user_atoms (MUX_user_data_type *puser_data, void  *handle);

/*!
   @brief   Get the meta data size

   @detail  Get the meta data size

   @param[in]  none

    @return    return metadata size.
*/
uint32 MUX_get_meta_data_size ();

/*!
   @brief  This is used to issue flush to the FileMUX.

   @detail   This is used to issue flush to the FileMUX.

*/
void MUX_Flush ();

/*!
   @brief  This is used to issue flush a particular straem to the FileMUX.

   @detail   This is used to issue flush a particular straem to the FileMUX.

*/
MUX_STATUS MUX_Flush (uint32 stream_id);

/*!
   @brief  This is used to update the status of the output buffers.

   @detail  This is used to update the status of the output buffers.

*/
void MUX_Output_Buffers_available(bool output_buffers_available);


/*!
   @brief  This is used to get the timestamp of the last muxed sample

   @detail  This is used to get the timestamp of the last muxed sample
   @Note    This function is not multithread safe and must be called
            from the callback thread for process sample

*/
void MUX_Get_Current_PTS(uint64 *pPTS);

private:
  //! The signal Q for the MUX working thread to wait on.
  MM_HANDLE m_signalQ;

  //! The thread exit signal. This signal is used to exit the MUX working thread.
  MM_HANDLE m_initMuxSignal;

  //! The thread exit signal. This signal is used to exit the MUX working thread.
  MM_HANDLE m_processSampleSignal;

  //! The thread exit signal. This signal is used to exit the MUX working thread.
  MM_HANDLE m_closeMuxSignal;

  MM_HANDLE m_exitSignal;

  MM_HANDLE m_flushBufferToFileSignal;

  //! The MUX working handle.
  MM_HANDLE m_MuxThreadHandle;

#ifdef FILEMUX_WRITE_ASYNC
  MM_HANDLE m_MuxFileThreadHandle;
  //! The signal Q for the MUX file write thread to wait on.
  MM_HANDLE m_fileSignalQ;

  MM_HANDLE m_exitFlushSignal;
#endif /*FILEMUX_WRITE_ASYNC*/
  MM_HANDLE m_MuxFileStatisticsHandle;
  MM_HANDLE m_statisticsSignalQ;
  MM_HANDLE m_getSpaceTimeSignal;
  MM_HANDLE m_exitStatisticsSignal;
  MM_HANDLE m_StatisticsTimerHandle;

  MUXCallbackFuncType m_MuxCallBackFunc;

  //! The MUX thread entry function.
  static int MuxThreadEntry( void* ptr );

  //! The MUX working thread entry function.
  int MuxThread( void );

  //! The MUX working thread priority.
  static const int MUX_THREAD_PRIORITY = 158;

  //! The signal Q to wait on close done.
  MM_HANDLE m_CloseDoneSignalQ;

  //! The MUX working thread stack size
  static const unsigned int MUX_THREAD_STACK_SIZE = 16384;


#ifdef FILEMUX_WRITE_ASYNC
  //! The MUX file thread entry function.
  static int MuxFileThreadEntry( void* ptr );

  //! The MUX file thread event handler.
  int MuxFileThread( void );

  static const int MUX_FILE_THREAD_PRIORITY = 9;

  static const unsigned int MUX_FILE_THREAD_STACK_SIZE = 8192;
#endif /*FILEMUX_WRITE_ASYNC*/

  //! The MUX file statistics entry function.
  static int MuxFileStatisticsEntry( void* ptr );

  //! The MUX file statistics event handler.
  int MuxFileStatistics( void );

  static const unsigned int MUX_FILE_STATISTICS_STACK_SIZE = 8192;

  void* m_pClientData;

  MUX_fmt_type m_file_format;

  MUX_brand_type m_file_brand;

  MUX_PEventData m_pEvent;

  FileMuxStatistics m_pMuxStatistics;
  //! Pointer to the base clss
  MUXBase *m_pMuxBase;

  //! Queue to hold the sample info and sample data pointers
  muxqueue* m_pSampleq;

  bool      m_bFlushStream[MUX_MAX_MEDIA_STREAMS];

  bool flush_issued;

  bool pause_issued;

  bool m_mux_closed;

  bool m_output_buffers_available;

  MUX_STATUS  m_eMuxStatus;
  //! Events the file source working thread processes.
  static const uint32 PROCESS_SAMPLE_EVENT;
  static const uint32 CLOSE_MUX_EVENT;
  static const uint32 THREAD_EXIT_EVENT;
#ifdef FILEMUX_WRITE_ASYNC
  static const uint32 PUSH_BUFFER_TO_FILE_EVENT;
  static const uint32 EXIT_FILE_THREAD_EVENT;
#endif /*FILEMUX_WRITE_ASYNC*/
  static const uint32 FILE_QUERY_SPACE_TIME_EVENT;
  static const uint32 EXIT_FILE_STATISTICS_EVENT;
};
#endif //__FILEMUX_H__
