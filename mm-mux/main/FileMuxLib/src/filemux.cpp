/* =======================================================================
							  FileMux.cpp
DESCRIPTION
  Definition of the muxbase class.
  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc., All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
							 Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/FileMuxLib/src/filemux.cpp#2 $

========================================================================== */

/* ==========================================================================

					 INCLUDE FILES FOR MODULE

========================================================================== */
#include "MMTimer.h"
#include "MMSignal.h"
#include "MMThread.h"
#include "MMDebugMsg.h"
#include "filemux.h"
#include "MuxQueue.h"
#include <threads.h>

//! Events the file source working thread processes.
const uint32 FileMux::PROCESS_SAMPLE_EVENT = 1;
const uint32 FileMux::CLOSE_MUX_EVENT = 2;
const uint32 FileMux::THREAD_EXIT_EVENT = 3;
#ifdef FILEMUX_WRITE_ASYNC
const uint32 FileMux::PUSH_BUFFER_TO_FILE_EVENT = 1;
const uint32 FileMux::EXIT_FILE_THREAD_EVENT = 2;
#endif
const uint32 FileMux::FILE_QUERY_SPACE_TIME_EVENT=1;
const uint32 FileMux::EXIT_FILE_STATISTICS_EVENT=2;

#define MM_MUX_THREAD_PRIORITY -14
#ifdef _ANDROID_
#define MM_Thread_ComponentPriority MM_Thread_DefaultPriority
#define MM_Thread_BackgroundPriority MM_Thread_DefaultPriority
#endif

/* ==========================================================================

				DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define DUMMY_BUFFER 1
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/*! ======================================================================
	@brief  FileMux constructor.

	@detail FileMux constructor.

=========================================================================*/
FileMux::FileMux(MUX_create_params_type *Params,
				 MUX_fmt_type file_format,
				 MUX_brand_type file_brand,
				 MUX_handle_type *output_handle,
				 boolean reorder_atom,
				 MUXCallbackFuncType callBack,
				 void *pClientData)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::FileMux");

  m_eMuxStatus = MUX_SUCCESS;
  mux_write_file_cb_func_ptr_type  pAysncCbFunc = NULL;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::FileMux");
  //! Assign callBack function from client user
  m_MuxCallBackFunc = callBack;
  m_pClientData = pClientData;
  m_file_format = file_format;
  m_file_brand = file_brand;
  m_pSampleq = NULL;
  m_signalQ = NULL;
  m_mux_closed = false;
#ifdef FILEMUX_WRITE_ASYNC
  m_fileSignalQ = NULL;
#endif /*FILEMUX_WRITE_ASYNC*/
  m_statisticsSignalQ = NULL;
  m_processSampleSignal = NULL;
  m_closeMuxSignal = NULL;
  m_exitSignal = NULL;
  m_flushBufferToFileSignal = NULL;
  m_MuxThreadHandle = NULL;
  m_getSpaceTimeSignal = NULL;
  m_StatisticsTimerHandle = NULL;
  m_CloseDoneSignalQ = NULL;
  m_MuxFileStatisticsHandle = NULL;
  m_exitStatisticsSignal = NULL;
#ifdef FILEMUX_WRITE_ASYNC
  if(file_format == MUX_FMT_MP4)
  {
	 pAysncCbFunc =  &(FileMux::callbackToWriteData);
  }
#endif /*#ifdef FILEMUX_WRITE_ASYNC*/

  m_pMuxBase =  MUXBase::CreateMuxInstance( Params
											,file_format
											,file_brand
											,output_handle
											,reorder_atom
#ifdef FILEMUX_WRITE_ASYNC
											,pAysncCbFunc,
											this
#endif /* FILEMUX_WRITE_ASYNC */
											);
  if( m_pMuxBase )
  {
	pause_issued = false;
	flush_issued = false;
	for(int i = 0; i < MUX_MAX_MEDIA_STREAMS; i++)
	{
	  m_bFlushStream[i] = false;
	}

	m_output_buffers_available = true;
	//! Create the signal Q for the thread to wait on.
	if ( 0 != MM_SignalQ_Create( &m_signalQ ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	//! Create the process sample signal. This signal is to process the audio/video/text/data samples.
	if ( (0 != MM_Signal_Create( m_signalQ,
								   (void *) &PROCESS_SAMPLE_EVENT,
								   NULL,
								   &m_processSampleSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}
	//! This signal is set to close the MUX and flush all the necessary data */
	if ( ( 0 != MM_Signal_Create( m_signalQ,
								   (void *) &CLOSE_MUX_EVENT,
								   NULL,
								   &m_closeMuxSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}
  /* Create the thread exit signal. This signal is set to exit the file open
   ** thread. */
	if (( 0 != MM_Signal_Create( m_signalQ,
								   (void *) &THREAD_EXIT_EVENT,
								   NULL,
								   &m_exitSignal ) ) )
  {
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	//! Create the the thread.
	if ( ( 0 != MM_Thread_CreateEx( 99, /*MUX_THREAD_PRIORITY*/
								   0,
								   FileMux::MuxThreadEntry,
								   this,
								   FileMux::MUX_THREAD_STACK_SIZE,
								   "Mux", &m_MuxThreadHandle) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
  }


  if(file_format == MUX_FMT_MP4)
  {
#ifdef FILEMUX_WRITE_ASYNC
	//! Create the signal Q for the thread to wait on.
	if ( 0 != MM_SignalQ_Create( &m_fileSignalQ ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

  //! Create the process sample signal. This signal is to process the audio/video/text/data samples.
	if ( ( 0 != MM_Signal_Create( m_fileSignalQ,
								   (void *) &PUSH_BUFFER_TO_FILE_EVENT,
								   NULL,
								   &m_flushBufferToFileSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	if ( ( 0 != MM_Signal_Create( m_fileSignalQ,
								   (void *) &EXIT_FILE_THREAD_EVENT,
								   NULL,
								   &m_exitFlushSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	//! Create the MuxFileAsync thread.
	if ( ( 0 != MM_Thread_CreateEx( MM_Thread_BackgroundPriority, /*MUX_FILE_THREAD_PRIORITY*/
								   0,
								   FileMux::MuxFileThreadEntry,
								   this,
								   FileMux::MUX_FILE_THREAD_STACK_SIZE,
								   "MuxFileAsync", &m_MuxFileThreadHandle) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}
#endif /* FILEMUX_WRITE_ASYNC */
	//! Create the signal Q for the statistics thread to wait on.
	if ( 0 != MM_SignalQ_Create( &m_statisticsSignalQ ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	if ( ( 0 != MM_Signal_Create( m_statisticsSignalQ,
								   (void *) &FILE_QUERY_SPACE_TIME_EVENT,
								   NULL,
								   &m_getSpaceTimeSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}
	if ( ( 0 != MM_Signal_Create( m_statisticsSignalQ,
								   (void *) &EXIT_FILE_STATISTICS_EVENT,
								   NULL,
								   &m_exitStatisticsSignal ) ) )
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

	//! Create the Mux File statistics thread.
	if ((0 != MM_Thread_CreateEx(MM_Thread_BackgroundPriority,
								0,
								FileMux::MuxFileStatisticsEntry,
								this,
								FileMux::MUX_FILE_STATISTICS_STACK_SIZE,
								"MuxFileStatistics", &m_MuxFileStatisticsHandle)))
	{
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}
   }
   else
   {
#ifdef FILEMUX_WRITE_ASYNC
	 m_fileSignalQ = NULL;
	 m_flushBufferToFileSignal = NULL;
	 m_exitFlushSignal = NULL;
	 m_MuxFileThreadHandle = NULL;
#endif //FILEMUX_WRITE_ASYNC
	 m_statisticsSignalQ = NULL;
	 m_exitStatisticsSignal = NULL;
	 m_MuxFileStatisticsHandle = NULL;
   }



	//! Create a queue
	m_pSampleq = new muxqueue;
	if( m_pSampleq == NULL )
	{
	   MUXBase::ReleaseMuxInstance(m_pMuxBase);
	   m_pMuxBase = NULL;
	   m_eMuxStatus = MUX_FAIL;
	   return;
	}

  }
  else
  {
	m_eMuxStatus = MUX_FAIL;
  }
}

/*! ======================================================================
	@brief  FileMux destructor.

	@detail FileMux destructor.

=========================================================================*/
FileMux::~FileMux()
{
   /* Thread exit code. */
  int exitCode = 0;

	 /**------------------------------------------------------------------------
  Delete the Mux statistics thread before killing mux thread
  ---------------------------------------------------------------------------
  */

  if(m_exitStatisticsSignal && m_MuxFileStatisticsHandle)
  {
	 int exitCode = 0;
	 MM_Signal_Set( m_exitStatisticsSignal );
	 MM_Thread_Join( m_MuxFileStatisticsHandle, &exitCode );
  }

  /* Send the exit signal to the source thread. */
  if(m_exitSignal && m_MuxThreadHandle)
  {
	MM_Signal_Set( m_exitSignal );
	MM_Thread_Join( m_MuxThreadHandle, &exitCode );
	/* Release the thread resources. */

  }
  else
  {
	if(m_pMuxBase)
	{
	  /**-----------------------------------------------------------------------
		  Mux thread exit called withuot closing Mux
	  --------------------------------------------------------------------------
	  */
	  if(!m_mux_closed)
	  {
		 (void)m_pMuxBase->MUX_end_Processing();
		 m_mux_closed = true;
	  }

	  MUXBase::ReleaseMuxInstance(m_pMuxBase);
	  m_pMuxBase = NULL;
	}
  }
#ifdef FILEMUX_WRITE_ASYNC
  /**------------------------------------------------------------------------
  Delete the Mux file thread before killing mux thread
  ---------------------------------------------------------------------------
  */

  if(m_exitFlushSignal && m_MuxFileThreadHandle)
  {
	 int exitCode = 0;
	 MM_Signal_Set( m_exitFlushSignal );
	 MM_Thread_Join( m_MuxFileThreadHandle, &exitCode );
  }
#endif /*FILEMUX_WRITE_ASYNC*/

  if(m_MuxThreadHandle)
  {
	MM_Thread_Release( m_MuxThreadHandle );
	m_MuxThreadHandle = NULL;
  }


  if(m_processSampleSignal)
  {
	MM_Signal_Release( m_processSampleSignal );
	m_processSampleSignal = NULL;
  }

  /* Release the close mux fail signal. */
  if(m_closeMuxSignal)
  {
	MM_Signal_Release( m_closeMuxSignal );
	m_closeMuxSignal = NULL;
  }

  /* Release the exit signal. */
  if(m_exitSignal)
  {
	MM_Signal_Release( m_exitSignal );
	m_exitSignal = NULL;
  }

  /* Release the signal Q. */
  if(m_signalQ)
  {
	MM_SignalQ_Release( m_signalQ );
	m_signalQ = NULL;
  }

#ifdef FILEMUX_WRITE_ASYNC
  /* Flush the buffer into file signal. */
  if(m_flushBufferToFileSignal)
  {
	MM_Signal_Release( m_flushBufferToFileSignal );
	m_flushBufferToFileSignal = NULL;
  }

  /* Exit Async Fileflush thread. */
  if(m_exitFlushSignal)
  {
	MM_Signal_Release( m_exitFlushSignal );
	m_exitFlushSignal = NULL;
  }

  if(m_getSpaceTimeSignal)
  {
	MM_Signal_Release( m_getSpaceTimeSignal );
	m_getSpaceTimeSignal = NULL;
  }
	/* Exit file statistics thread. */
  if(m_exitStatisticsSignal)
  {
	MM_Signal_Release( m_exitStatisticsSignal );
	m_exitStatisticsSignal = NULL;
  }


  /* Release the signal Q. */
  if(m_fileSignalQ)
  {
	MM_SignalQ_Release( m_fileSignalQ );
	m_fileSignalQ = NULL;
  }
  if(m_MuxFileThreadHandle)
  {
	MM_Thread_Release( m_MuxFileThreadHandle );
	m_MuxFileThreadHandle = NULL;
  }

#endif /* FILEMUX_WRITE_ASYNC  */


  if(m_statisticsSignalQ)
  {
	MM_SignalQ_Release( m_statisticsSignalQ );
	m_statisticsSignalQ = NULL;
  }

  if(m_MuxFileStatisticsHandle)
  {
	MM_Thread_Release( m_MuxFileStatisticsHandle );
	m_MuxFileStatisticsHandle = NULL;
  }

  if(m_pMuxBase)
  {
	MUXBase::ReleaseMuxInstance(m_pMuxBase);
	m_pMuxBase = NULL;
  }

  if(m_pSampleq)
  {
	delete m_pSampleq;
	m_pSampleq = NULL;
  }
}

/*! ======================================================================
	@brief  Get the Mux status

	@detail This function read the current status of Muxer which will be
			the status of last completed operation on Muxer.

	@param[in] void

	@return     MUX_STATUS.
=========================================================================*/
MUX_STATUS FileMux::MUX_get_Status()
{
  return m_eMuxStatus;
}

/*! ======================================================================
	@brief  call to end the MUX processing

	@detail call to end the MUX processing

	@param[in]  pClientData context data

	@return     MUX_STATUS.
=========================================================================*/
MUX_STATUS FileMux::MUX_end_Processing(void *pClientData)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MUX_end_Processing");
  m_pClientData =  pClientData;
  if(m_closeMuxSignal)
  {
	MM_Signal_Set( m_closeMuxSignal );
  }
  m_eMuxStatus = MUX_SUCCESS;
  return m_eMuxStatus;
}

/*! ======================================================================
	@brief  call to pause the MUX processing

	@detail call to pause the MUX processing

	@param[in]  pClientData context data

	@return     MUX_STATUS.
=========================================================================*/
MUX_STATUS FileMux::MUX_pause_Processing(void * /*pClientData*/)
{
  pause_issued = true;
  if( m_pSampleq )
  {
	if(m_pSampleq->queue_size() == 0)
	{
	 /**---------------------------------------------------------------
	   Queue size can be zero if no sample is either queued or
	   when a sample is popped from queue and is beng processed.
	   Here we set a signal to process sample so that we can get the
	   sample being processed also completed.
	 ------------------------------------------------------------------
	 */
	  MM_Signal_Set( m_processSampleSignal );
	}
  }
  m_eMuxStatus = MUX_SUCCESS;
  return m_eMuxStatus;
}


/*! ======================================================================
	@brief  call to resume the MUX processing

	@detail call to resume the MUX processing

	@param[in]  pClientData context data

	@return     MUX_STATUS.
=========================================================================*/
MUX_STATUS FileMux::MUX_resume_Processing(void * /*pClientData*/)
{
  if(pause_issued != true)
  {
	  /**-----------------------------------------------------------------
		 Not is pause.Spurious command from client
	  --------------------------------------------------------------------
	  */
	  m_eMuxStatus = MUX_INVALID;
	  return m_eMuxStatus;
  }
  pause_issued = false;
  /**---------------------------------------------------------------------
	 Lets set the signal to process samples if any.
  ------------------------------------------------------------------------
  */
  MM_Signal_Set( m_processSampleSignal );
  m_eMuxStatus = MUX_SUCCESS;
  return m_eMuxStatus;
}

/*! ======================================================================
	@brief  call to process audio/video samples

	@detail call to process audio/video samples

	@param[in]  stream_number audio/video stream number
	@param[in]  num_samples num of samples
	@param[in]  sample_info sample info
	@param[in]  sample_data actual sample data
	@param[in]  pClientData context info
	@return     MUX_STATUS.

=========================================================================*/
MUX_STATUS FileMux::MUX_Process_Sample( uint32 stream_number,
								 uint32 num_samples,
								 const MUX_sample_info_type  *sample_info,
								 const uint8  *sample_data,
								 void *pClientData)
{
  MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_MEDIUM,
			  "FileMux::MUX_Process_Sample stream_number %d", stream_number);

  if(stream_number >= MUX_MAX_MEDIA_STREAMS)
  {
	  /**------------------------------------------------------------------
		 Not expecting samples in paused state.
	  ---------------------------------------------------------------------
	  */
	  m_eMuxStatus = MUX_INVALID;
	  return m_eMuxStatus;
  }
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  ////Push the sample into the queue
  if(m_pSampleq)
  {
	if(!m_pSampleq->Push(stream_number,false, num_samples,sample_info,sample_data, pClientData))
	{
		/* Sample queue full */
		m_eMuxStatus = MUX_QUEUE_FULL;
		return m_eMuxStatus;
	}

	// Set signal to process the sample
	if(!pause_issued)
	{
		MM_Signal_Set( m_processSampleSignal );
	}
	m_eMuxStatus = MUX_SUCCESS;
	return m_eMuxStatus;
  }
  else
  {
	m_eMuxStatus = MUX_NOTAVAILABLE;
	return m_eMuxStatus;
  }
}

/*! ======================================================================
	@brief  ctreate filemux thread.

	@detail The file source working thread entry function. Once the thread is created the control
			comes to this function.

	@param[in]  ptr  the "this" pointer.
	@return     status.

	@note   It is expected that when this function is called the file open thread has been
			created.
=========================================================================*/
int FileMux::MuxThreadEntry( void* ptr )
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry");
  FileMux* pThis = (FileMux *) ptr;

  if ( NULL != pThis )
  {
	pThis->MuxThread();
  }
  return 0;
}

/*! ======================================================================
	@brief  MUX thread entry function.

	@detail The MUX working thread which handles commands and events posted to it.

	@param[in]  none.
	@return     none.
=========================================================================*/
int FileMux::MuxThread( void )
{
  bool bRunning = true;
  MUX_flush_info_type sFlushInfo;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThread");

  int tid = androidGetTid();
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "WFDD: MuxThread  priority b4 %d ", androidGetThreadPriority(tid));
  androidSetThreadPriority(0,MM_MUX_THREAD_PRIORITY);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "WFDD:MuxThread  priority after%d ", androidGetThreadPriority(tid));

  while ( bRunning )
  {
	/* Wait for a signal to be set on the signal Q. */
	uint32 *pEvent = NULL;
	struct mux_item *item = NULL;
	if ( 0 == MM_SignalQ_Wait( m_signalQ, (void **) &pEvent ) )
	{
	  switch ( *pEvent )
	  {
		case PROCESS_SAMPLE_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "FileMux::MuxThreadEntry PROCESS_SAMPLE_EVENT");
		  do
		  {

			if(pause_issued == true)
			{
			  /**---------------------------------------------------------------
				Issueing pause complete here just makes sure that no sample is
				in the middle of processing, however it is unlikely.
			  ------------------------------------------------------------------
			  */
			  m_MuxCallBackFunc(PAUSE_COMPLETED, m_pClientData, NULL, NULL);

			  /**---------------------------------------------------------------
				Lets not process anything here as client has paused.
			  ------------------------------------------------------------------
			  */
			  break;
			}
			if(m_output_buffers_available)
			{
			  if( m_pSampleq )
			  {
				//pop the sample from the queue
				item = m_pSampleq->Pop_Front();
				if(item)
				{
				  if(flush_issued && m_bFlushStream[item->m_stream_number])
				  {
					/**--------------------------------------------------------
					  Client wants to flush all samples in this
					  stream
					 -----------------------------------------------------------
					 */
					m_MuxCallBackFunc(PROCESS_SAMPLE_FLUSH,
									  m_pClientData, (void *)item->m_sample_info,
									   item->m_ptr);
					continue;
				  }
				  if( m_pMuxBase )
				  {
					if(item->m_bheader == true)
					{
					  m_eMuxStatus = m_pMuxBase->MUX_write_header(item->m_stream_number,
															item->m_num_samples,
															item->m_sample_data);
					  if(m_eMuxStatus == MUX_SUCCESS)
					  {
						 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
											"FileMux::MuxThreadEntry Header Processed Asynch.ly");
						 m_MuxCallBackFunc(PROCESS_HEADER_COMPLETE,
							 m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					  }
					  else
					  {
						 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
											"FileMux::MuxThreadEntry Header Failed Process Asynch.ly");
						 m_MuxCallBackFunc(PROCESS_HEADER_FAIL,
							 m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					  }
					  continue;
					}
					//Procees the sample
					if(item->m_sample_info->size)
					{
					  m_eMuxStatus = m_pMuxBase->MUX_Process_Sample(item->m_stream_number, item->m_num_samples, item->m_sample_info, item->m_sample_data);
					}
					else
					{
					  //If buffer is empty just return complete
					  m_eMuxStatus = MUX_SUCCESS;
					}
				  }
				  if(m_eMuxStatus == MUX_SUCCESS)
				  {
					m_MuxCallBackFunc(PROCESS_SAMPLE_COMPLETE, m_pClientData, (void *)item->m_sample_info, item->m_ptr);
				  }
				  else if(m_eMuxStatus == MUX_SPACE_LIMIT_REACHED)
				  {
					m_MuxCallBackFunc(SPACE_LIMIT_REACHED, m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
											"FileMux::MuxThreadEntry MUX_SPACE_LIMIT_REACHED");
				  }
				  else if(m_eMuxStatus == MUX_WRITE_FAILED)
				  {
					m_MuxCallBackFunc(WRITE_FAILED, m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
											"FileMux::MuxThreadEntry MUX_WRITE_FAILED");
				  }

				  else if(m_eMuxStatus == MUX_OUTDATED)
				  {
					m_MuxCallBackFunc(PROCESS_SAMPLE_OUTDATED, m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
											"FileMux::MuxThreadEntry PROCESS_SAMPLE_EVENT failed Outdated buffer");
					m_eMuxStatus = MUX_SUCCESS;
				  }
				  else
				  {
					m_MuxCallBackFunc(PROCESS_SAMPLE_FAIL, m_pClientData, (void *)item->m_sample_info, item->m_ptr);
					MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
											"FileMux::MuxThreadEntry PROCESS_SAMPLE_EVENT failed");
				  }
				}
			  }
			}
		  }while(item != NULL);
		  if(flush_issued)
		  {
			uint32 num_streams = 0;
			for(int i = 0; i < MUX_MAX_MEDIA_STREAMS; i++)
			{
			  /**---------------------------------------------------------------
				Lets give one callback each for each stream flushed
			  ------------------------------------------------------------------
			  */
			  if(m_bFlushStream[i])
			  {
				 num_streams ++;
				 sFlushInfo.stream_id = i;
				 m_MuxCallBackFunc(FLUSH_COMPLETED, m_pClientData, (void*)&sFlushInfo ,NULL);
			  }
			  m_bFlushStream[i] = false;
			}
			if(!num_streams)
			{
			   m_MuxCallBackFunc(FLUSH_COMPLETED, m_pClientData, NULL, NULL);
			}

			MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "FileMux::MuxThreadEntry FLUSH_COMPLETED");
			flush_issued = false;
		  }
		  break;
		}
		case CLOSE_MUX_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry CLOSE_MUX_EVENT");
		  /* If there are any buffers while closing we need to release here*/

		  uint32 queue_size = 0;

		  if(m_pSampleq)
		  {
			m_pSampleq->queue_size();
		  }
		  if(queue_size != 0)
		  {
			  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry Flush held buffers");
			  while(queue_size)
			  {
				  queue_size--;
				  if(m_pSampleq)
				  {
					item = m_pSampleq->Pop_Front();
				  }
				  else
				  {
					item = NULL;
				  }
				  if(!item)
				  {
					 continue;
				  }
				  m_MuxCallBackFunc(PROCESS_SAMPLE_FLUSH,
							  m_pClientData, (void *)item->m_sample_info,
							  item->m_ptr);
			  }
		  }
		  if( !m_mux_closed && m_pMuxBase )
		  {
			m_pMuxBase->MUX_write_footer(1, 1, NULL);
			m_eMuxStatus = m_pMuxBase->MUX_end_Processing();
			m_mux_closed = true;
		  }


		  if(m_eMuxStatus == MUX_SUCCESS)
		  {
			m_MuxCallBackFunc(CLOSE_MUX_COMPLETE, m_pClientData, NULL, NULL);
		  }
		  else
		  {
			m_MuxCallBackFunc(CLOSE_MUX_FAIL, m_pClientData, NULL, NULL);
			MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry CLOSE_MUX_EVENT failed");
		  }
		  break;
		}
		case THREAD_EXIT_EVENT:
		{
		   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry received THREAD_EXIT_EVENT");
		   if(m_pMuxBase)
		   {
			  /**-----------------------------------------------------------------------
				  Mux thread exit called withuot closing Mux
			  --------------------------------------------------------------------------
			  */
			  if(!m_mux_closed)
			  {
				  (void)m_pMuxBase->MUX_end_Processing();
				  m_mux_closed = true;
			  }


			  MUXBase::ReleaseMuxInstance(m_pMuxBase);
			  m_pMuxBase = NULL;
		  }

		  /* Exit the thread. */
		  bRunning = false;
		  MM_Thread_Exit( FileMux::m_MuxThreadHandle, 0 );
		  break;
		}
		default:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry received UNKNOWN EVENT");
		  /* Not a recognized event, ignore it. */
		}
	  }
	}
  }
  return 0;
}

#ifdef FILEMUX_WRITE_ASYNC
/*! ======================================================================
	@brief  ctreate filemux file async thread.

	@detail The file mux file writing thread entry point.

	@param[in]  ptr  the "this" pointer.
	@return     status.

	@note   It is expected that when this function is called the file open thread has been
			created.
=========================================================================*/
int FileMux::MuxFileThreadEntry( void* ptr )
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThreadEntry");
  FileMux* pThis = (FileMux *) ptr;

  if ( NULL != pThis )
  {
	pThis->MuxFileThread();
  }
  return 0;
}

/*! ======================================================================
	@brief  MUX thread entry function.

	@detail The MUX working thread which handles commands and events posted to it.

	@param[in]  none.
	@return     none.
=========================================================================*/
int FileMux::MuxFileThread( void )
{
  bool bRunning = true;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxThread");
  while ( bRunning )
  {
	/* Wait for a signal to be set on the signal Q. */
	uint32 *pEvent = NULL;

	if ( 0 == MM_SignalQ_Wait( m_fileSignalQ, (void **) &pEvent ) )
	{
	  switch ( *pEvent )
	  {
		case PUSH_BUFFER_TO_FILE_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileThreadEntry received PUSH_BUFFER_TO_FILE_EVENT");
		  if(m_pMuxBase)
		  {
			/**-----------------------------------------------------------------------
				 Mux thread exit called withuot closing Mux
			--------------------------------------------------------------------------
			*/
			m_pMuxBase->FlushStream();
		  }
		}
		break;
		case EXIT_FILE_THREAD_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileThreadEntry received EXIT_FILE_THREAD_EVENT");
		  bRunning = false;
		  MM_Thread_Exit( FileMux::m_MuxFileThreadHandle, 0 );
		}
		break;
		default:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileThreadEntry received UNKNOWN EVENT");
		  /* Not a recognized event, ignore it. */
		}
	  }
	}
  }
  return 0;
}
#endif /*FILEMUX_WRITE_ASYNC*/

/*! ======================================================================
	@brief  ctreate filemux file statistics thread.

	@detail The file mux statistics thread entry point.

	@param[in]  ptr  the "this" pointer.
	@return     status.

	@note   It is expected that when this function is called the file open thread has been
			created.
=========================================================================*/
int FileMux::MuxFileStatisticsEntry( void* ptr )
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileStatisticsEntry");
  FileMux* pThis = (FileMux *) ptr;

  if ( NULL != pThis )
  {
	pThis->MuxFileStatistics();
  }
  return 0;
}


/*! ======================================================================
	@brief  MUX statistics thread event handler.

	@detail The MUX working thread which handles queries by IL client regarding available
			   space/time during recording.

	@param[in]  none.
	@return     none.
=========================================================================*/
int FileMux::MuxFileStatistics( void )
{
  bool bRunning = true;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileStatistics");
  while ( bRunning )
  {
	/* Wait for a signal to be set on the signal Q. */
	uint32 *pEvent = NULL;

	if ( 0 == MM_SignalQ_Wait( m_statisticsSignalQ, (void **) &pEvent ) )
	{
	  switch ( *pEvent )
	  {
		case FILE_QUERY_SPACE_TIME_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileStatistics received FILE_QUERY_SPACE_TIME_EVENT");

		  if(m_pMuxBase && !m_mux_closed)
		  {
		/**---------------------------------------------------------------------
			 Query EFS and video fmt writer for recording/available space/size
		------------------------------------------------------------------------
		*/
			 m_pMuxBase->MUX_get_Statistics(&m_pMuxStatistics);
		/**---------------------------------------------------------------------
			 * Give callback to mmi layer o that it can notify client
			 * to query for the statistics data.
		------------------------------------------------------------------------
		*/
			 m_MuxCallBackFunc(MUX_STATISTICS, m_pClientData, (void *)&m_pMuxStatistics, NULL);
		  }
		}
		break;
		case EXIT_FILE_STATISTICS_EVENT:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileStatistics received EXIT_FILE_STATISTICS_EVENT");
		  bRunning = false;
		  if(m_StatisticsTimerHandle != NULL)
		  {
			MM_Timer_Release(m_StatisticsTimerHandle);
			m_StatisticsTimerHandle = NULL;
		  }
		  MM_Thread_Exit( FileMux::m_MuxFileStatisticsHandle, 0 );
		}
		break;
		default:
		{
		  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MuxFileStatistics received UNKNOWN EVENT");
		  /* Not a recognized event, ignore it. */
		}
	  }
	}
  }
  return 0;
}

/*!
   @brief   Write the header to the movie file

   @detail  Write the header to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
			   audio_header_size - Length of the footer
			   audio_header - audio headter pointer
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_write_header (uint32 audio_str, uint32 audio_header_size, const uint8 *audio_header)
{
	if(!m_pMuxBase)
	{
	   m_eMuxStatus = MUX_FAIL;
	   return m_eMuxStatus;
	}
	m_eMuxStatus = m_pMuxBase->MUX_write_header(audio_str, audio_header_size, audio_header);
	return m_eMuxStatus;
}

/*!
   @brief   Write the header to the movie file

   @detail  Write the header to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
			   bAsync        - To process synchronously or Asynchronously
			   audio_header_size - Length of the footer
			   audio_header - audio headter pointer
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_write_header (uint32 str_num,bool bAsync, uint32 header_size,
									const uint8 *pheader, void *pClientData)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  if(!bAsync)
  {
	m_eMuxStatus = m_pMuxBase->MUX_write_header(str_num, header_size, pheader);
	return m_eMuxStatus;
  }
  if(m_pSampleq)
  {
	if(!m_pSampleq->Push(str_num,true, header_size,NULL,pheader, pClientData))
	{
		/* Sample queue full */
	   m_eMuxStatus = MUX_QUEUE_FULL;
	   return m_eMuxStatus;
	}

	// Set signal to process the header
	if(!pause_issued)
	{
		MM_Signal_Set( m_processSampleSignal );
	}
	m_eMuxStatus = MUX_SUCCESS;
  }
  else
  {
	m_eMuxStatus = MUX_NOTAVAILABLE;
  }
  return m_eMuxStatus;
}

/*!
   @brief   Write the footer to the movie file

   @detail  Write the footer to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
			   footer_length - Length of the footer
			   buf_ptr - Buffer pointer
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_write_footer (uint32 stream_num, uint32 footer_length, const uint8 *buf_ptr)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_write_footer(stream_num, footer_length, buf_ptr);
  return m_eMuxStatus;
}

/*!
   @brief   Write the meta data

   @detail  Set one of the many meta data tag info supported

   @param[in]  FileMuxMetaDataType Type of meta data
   @param[in]  FileMuxMetaData  associated data
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_set_MetaData(FileMuxMetaDataType eType,
							FileMuxMetaData *pMetaData)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }

  return m_pMuxBase->MUX_set_MetaData(eType, pMetaData);
}

/*!
   @brief   timer handler function

   @detail  timer handler function for statistics query

	@return    void.
*/

void FileMux::MUX_StatisticsInterval_TimerHandler (void * pMuxStatistics)
{
	FileMux *pThis;
	pThis = (FileMux *)pMuxStatistics;
	pThis->MUX_get_Statistics();
}


/*!
   @brief   set statistics query  timer interval

   @detail  set statistics query  timer interval

   @param[in]  OMX_TICKS time interval
	@return    MUX_STATUS.
*/

MUX_STATUS FileMux::MUX_set_StatisticsInterval(uint64 interval)
{
	if(m_StatisticsTimerHandle != NULL)
	{
	  MM_Timer_Release(m_StatisticsTimerHandle);
	  m_StatisticsTimerHandle = NULL;
	}
	if(0 != MM_Timer_Create((int)interval, 1, FileMux::MUX_StatisticsInterval_TimerHandler,
							  (void *)this, &m_StatisticsTimerHandle))
	{
	  m_eMuxStatus = MUX_FAIL;
	  return m_eMuxStatus;
	}
	return MUX_SUCCESS;
}

/*!
   @brief   Get the Recorded/Available space/time from ISO basefile

   @detail  Get the Recorded/Available space/time from ISO basefile

   @param[in]  FileMuxMetaDataType Type of meta data
   @param[in]  FileMuxMetaData  associated data
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_get_Statistics()
{
  if (0!= MM_Signal_Set( m_getSpaceTimeSignal))
  {
	  m_eMuxStatus = MUX_FAIL;
  }
  else
  {
	  m_eMuxStatus = MUX_SUCCESS;
  }
  return m_eMuxStatus;
}


/*!
   @brief   Write the header text movie file

   @detail  Write the header text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
			   entry - Text to write
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_write_header_text (uint32 stream_num, const MUX_text_type_t *entry)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_write_header_text(stream_num, entry);
  return m_eMuxStatus;
}

/*!
   @brief   Write the text sample to movie file

   @detail  Write the text text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
			   handle -
			   delta -
	@return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_write_text (uint32 stream_num, void * handle, uint32 delta)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_write_text(stream_num, handle, delta);
  return m_eMuxStatus;
}

/*!
   @brief  To release a timed text stream sample without writing it to the stream.

   @detail  To release a timed text stream sample without writing it to the stream.

   @param[in]  handle - To Release

   @return    MUX_STATUS.
*/

MUX_STATUS FileMux::MUX_free_text (void * handle)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_free_text(handle);
  return m_eMuxStatus;
}

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
MUX_STATUS FileMux::MUX_write_uuid (const uint8 *uuid, const void *data, uint32 size)
{
   if(!m_pMuxBase)
   {
	 m_eMuxStatus = MUX_FAIL;
	 return m_eMuxStatus;
   }
   m_eMuxStatus = m_pMuxBase->MUX_write_uuid(uuid, data, size);
   return m_eMuxStatus;
}

/*!
   @brief  This is used to update the mehd with total duration.

   @detail   This is used to update the mehd with total duration.

   @param[in] total_duration - total duration to be written.

   @return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_update_mehd_atom (uint32 total_duration)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_update_mehd_atom(total_duration);
  return m_eMuxStatus;
}

/*!
   @brief  This is used to update the port instance for streaming.

   @param[in] streamport - instance of IStreamport

   @return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_update_streamport (uint64_t streamport)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
							"Filemux::Update Stream Port");
  m_eMuxStatus = m_pMuxBase->MUX_update_streamport(streamport);
  return m_eMuxStatus;
}

/*!
   @brief  This is used to update the UDTA atom with the given data

   @detail    This is used to update the UDTA atom with the given data

   @param[in] data - data to be updated with.
			  size - size of the data.

   @return    MUX_STATUS.
*/

MUX_STATUS FileMux::MUX_write_user_meta_data (const void *data, uint32 size)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_write_user_meta_data(data, size );
  return m_eMuxStatus;
}

/*!
   @brief  Mux given data in every timeout to the output handler

   @detail  Mux given data in every timeout to the output handler

   @param[in] data - data to be updated with.
              size - size of the data.
              timeout - the timeout of the write call back function in millseconds
   @return    MUX_STATUS.
*/

MUX_STATUS FileMux::MUX_write_user_meta_data (const void *data, uint32 size, uint32 timeout)
{
  if(!m_pMuxBase)
  {
    m_eMuxStatus = MUX_FAIL;
    return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_write_user_meta_data(data, size,timeout);
  return m_eMuxStatus;
}
/*!
   @brief  This is used to update the clipinfo.

   @detail   This is used to update the clipinfo.

   @param[in] msg_list - list of messages with clip info.

   @return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_update_clipinfo (MUX_msg_list_type *msg_list)
{
  if(!m_pMuxBase)
  {
	 m_eMuxStatus = MUX_FAIL;
	 return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_update_clipinfo(msg_list);
  return m_eMuxStatus;
}

/*!
   @brief  This is used to update AVC Timing and HRD params.

   @detail   Params used to populate the Timing and
			HRD descriptor in AVC is set using this function.

   @param[in] pHRDParams - pointer to Timing and HRD params structure.

   @return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_update_AVC_Timing_HRD_Params(MUX_AVC_TimingHRD_params_type *pHRDParams)
{
  if(!m_pMuxBase)
  {
	 return MUX_FAIL;
  }

  return m_pMuxBase->MUX_update_AVC_Timing_HRD_Params(pHRDParams);
}

/*!
   @brief  This is used to modify user data atoms.

   @detail   This is used to modify user data atoms.

   @param[in] puser_data - user data atom to be modified.
			  handle   -  handle

   @return    MUX_STATUS.
*/
MUX_STATUS FileMux::MUX_modify_user_atoms (MUX_user_data_type *puser_data, void  *handle)
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return m_eMuxStatus;
  }
  m_eMuxStatus = m_pMuxBase->MUX_modify_user_atoms(puser_data, handle);
  return m_eMuxStatus;
}

/*!
   @brief   Get the meta data size

   @detail  Get the meta data size

   @param[in]  none

	@return    return metadata size.
*/
uint32 FileMux::MUX_get_meta_data_size ()
{
  if(!m_pMuxBase)
  {
	m_eMuxStatus = MUX_FAIL;
	return 0;
  }
  return m_pMuxBase->MUX_get_meta_data_size();
}


/*!
   @brief  This is used to issue flush to the FileMUX.

   @detail   This is used to issue flush to the FileMUX for a particular stream.

*/
MUX_STATUS FileMux::MUX_Flush (uint32 stream_id)
{
  struct mux_item *item;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MUX_Flush");

  if(stream_id >= MUX_MAX_MEDIA_STREAMS)
  {
	  m_eMuxStatus = MUX_INVALID;
	  return m_eMuxStatus;
  }
  if(pause_issued == true)
  {
	  /**--------------------------------------------------------------
		We are in pause, so no buffer is being processed. We need to
		flush the queue here.
	  -----------------------------------------------------------------
	  */
	  uint32 queue_size = m_pSampleq->queue_size();
	  if(queue_size != 0)
	  {
		  while(queue_size)
		  {
			  queue_size--;
			  item = m_pSampleq->Pop_Front();
			  if(!item)
			  {
				  continue;
			  }
			  if(item->m_stream_number != stream_id)
			  {
				  m_pSampleq->Push(item->m_stream_number,item->m_bheader, item->m_num_samples,item->m_sample_info,item->m_sample_data, item->m_ptr);
			  }
			  else
			  {
				  m_MuxCallBackFunc(PROCESS_SAMPLE_FLUSH,
					  m_pClientData, (void *)item->m_sample_info,
					   item->m_ptr);
			  }
		  }
	  }
	  m_eMuxStatus = MUX_DONE;
	  return m_eMuxStatus;
  }


  flush_issued = true;
  m_bFlushStream[stream_id] = true;
  /**------------------------------------------------------------------
	 If we have nothing to flush just post an event to
	 m_processSampleSignal and send flush complete there.

  ---------------------------------------------------------------------
  */
  if(m_pSampleq->queue_size() == 0)
  {
	 /**---------------------------------------------------------------
	   Queue size can be zero if no sample is either queued or
	   when a sample is popped from queue and is beng processed.
	   Here we set a signal to process sample so that we can get the
	   sample being processed also completed.
	 ------------------------------------------------------------------
	 */
	 MM_Signal_Set( m_processSampleSignal );
  }
  m_eMuxStatus = MUX_SUCCESS;
  return m_eMuxStatus;
}

/*!
   @brief  This is used to issue flush to the FileMUX.

   @detail   This is used to issue flush to the FileMUX.

*/
void FileMux::MUX_Flush ()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MUX_Flush");
  flush_issued = true;
  if(m_pSampleq->queue_size() == 0)
  {
	// There are no samples pending in the queue so issue FLUSH completed
	m_MuxCallBackFunc(FLUSH_COMPLETED, m_pClientData, NULL, NULL);
  }
  m_eMuxStatus = MUX_SUCCESS;
}

/*!
   @brief  This is used to get the timestamp of the last muxed sample

   @detail  This is used to get the timestamp of the last muxed sample

*/
void FileMux::MUX_Get_Current_PTS(uint64 *pPTS)
{
    if (m_pMuxBase)
    {
        m_pMuxBase->MUX_get_current_PTS(pPTS);
    }
}

/*!
   @brief  This is used to update the status of the output buffers.

   @detail  This is used to update the status of the output buffers.

*/
void FileMux::MUX_Output_Buffers_available(bool output_buffers_available)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "FileMux::MUX_Output_Buffers_available");
  m_output_buffers_available = output_buffers_available;
  // There are no samples pending in the queue so issue process signal
  if((m_output_buffers_available) && (m_pSampleq->queue_size() > 0))
  {
	// Set signal to process the sample
	MM_Signal_Set( m_processSampleSignal );
  }
}

#ifdef FILEMUX_WRITE_ASYNC
/*!
   @brief  callback function start flushing data into file.

   @detail  callback function start flushing data into file.

*/
  void FileMux::callbackToWriteData(
	  void             *client_data  //! Client data from calling function
)
{
	FileMux* pThis = NULL;
	if(client_data)
	{
	  pThis = (FileMux *) client_data;
	  MM_Signal_Set( pThis->m_flushBufferToFileSignal );
	}
}
#endif  /* FILEMUX_WRITE_ASYNC  */



