#ifndef __MUX_BASE_H__
#define __MUX_BASE_H__

/* =======================================================================
                              muxbase.h
DESCRIPTION
  This is base class for MUX.

  Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/MuxBaseLib/inc/muxbase.h#2 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#include "filemuxinternaldefs.h"
#include "filemuxtypes.h"
#include "isucceedfail.h"
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* This enumerated type is used to select a file format. */
/*!
  @brief   MUXBase Module.

  @detail The Intent of having this module is to Provide a common interface for
          upperlayers to control the MUX (for both MP4, AVI etc.. file formats)
		  and mux the audio/video/text/data streams into single stream and generate
		  necessary meta data and write into the application supplied
		  output port/flash/MMC/memory/client. It also manages the reordering
		  and temporary storage management if necessary.


  @note   Inorder to use the functions provided by this class. It is required
          that the User calls CreateMuxInstance before calling others
*/
class MUXBase  : public ISucceedFail
{
public:
	//! MUXBase destructor gets called when delete the interface
    virtual ~MUXBase(){};

    /**************************************************************************
     ** Public Methods
     *************************************************************************/
public:
/* Creates MUX instance */
/*!
   @brief   Creates a MUX instance

   @detail  It takes file_format, file_brand, output_handle, notification callBack and pOputStream as input parameters.
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
static MUXBase * CreateMuxInstance (MUX_create_params_type *Params
                  , MUX_fmt_type file_format
                  , MUX_brand_type file_brand
                  , MUX_handle_type *output_handle
                  , boolean reorder_atom
#ifdef FILEMUX_WRITE_ASYNC
                  , mux_write_file_cb_func_ptr_type file_write_fn
                  , void * client_data
#endif   /*    FILEMUX_WRITE_ASYNC  */
				  );

/*!
   @brief   Releases a MUX instance

   @detail  Releases the AAC or EVRC of MP4 mux base instance

   @param[in]  pointer to the Mux base instance.

   @return     void.

   @note    It is expected that except pOputStream nothing should be NULL.
   */
static void ReleaseMuxInstance (void *pMux);

/*!
   @detail   Stop the MUX

   @detail   Done recording so stop the processing.

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_end_Processing(){return MUX_SUCCESS;};
/*!
   @brief   Processes the given audio/video/text/data sample

   @detail  It takes audio/video/text/data sample and processes.

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               num_samples - num of samples
			   sample_info - Sample info
			   sample_data - Actual sample data.
    @return    MUX_STATUS.

   @note    It is expected that except pOputStream nothing should be NULL.
   */

virtual MUX_STATUS MUX_Process_Sample( uint32 /*stream_number*/,
                                 uint32 /*num_samples*/,
                                 const MUX_sample_info_type  * /*sample_info*/,
                                 const uint8  * /*sample_data*/){return MUX_SUCCESS;};
/*!
   @brief   Write the header to the movie file

   @detail  Write the header to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               audio_header_size - Length of the footer
			   audio_header - audio headter pointer
    @return    MUX_STATUS.
*/

virtual MUX_STATUS MUX_write_header (uint32 /*audio_str*/,
                                     uint32 /*audio_header_size*/,
                                     const uint8 * /*audio_header*/){return MUX_SUCCESS;};

/*!
   @brief   Write the footer to the movie file

   @detail  Write the footer to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               footer_length - Length of the footer
			   buf_ptr - Buffer pointer
    @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_write_footer (uint32 /*stream_num*/,
                                     uint32 /*footer_length*/,
                                     const uint8 * /*buf_ptr*/){return MUX_SUCCESS;};
/*!
   @brief   Write the header text movie file

   @detail  Write the header text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               entry - Text to write
    @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_write_header_text (uint32 /*stream_num*/,
                                          const MUX_text_type_t * /*entry*/){return MUX_SUCCESS;};
/*!
   @brief   Write the text sample to movie file

   @detail  Write the text text to the movie file

   @param[in]  stream_number - Steram type can be either audio/video/text/data
               handle -
			   delta -
    @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_write_text (uint32 /*stream_num*/,
                                   void * /*handle*/,
                                   uint32 /*delta*/){return MUX_SUCCESS;};

/*!
   @brief  To release a timed text stream sample without writing it to the stream.

   @detail  To release a timed text stream sample without writing it to the stream.

   @param[in]  handle - To Release

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_free_text (void * /*handle*/){return MUX_SUCCESS;};
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
virtual MUX_STATUS MUX_write_uuid (const uint8 * /*uuid*/,
                                   const void * /*data*/,
                                   uint32 /*size*/){return MUX_SUCCESS;};

/*!
   @brief  This is used to pass user data atom information to be written to
           the header of the movie file.

   @detail  This is used to pass user data atom information to be written to
            the header of the movie file.

   @param[in]
              data - Data
              size - Size

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_write_user_meta_data(const void * /*pdata*/,
                                            uint32 /*nsize*/){return MUX_SUCCESS;};
/*!
   @brief  Mux user meta data information in every timeout

   @detail    Mux user meta data information in every timeout

   @param[in] data - data to be updated with.
              size - size of the data.
              timeout - the timeout of the write callback function (in millisecond)

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_write_user_meta_data (const void * /*data*/,
                                             uint32 /*size*/,
                                             uint32 /*timeout*/){return MUX_SUCCESS;};
/*!
   @brief  This is used to update the mehd with total duration.

   @detail   This is used to update the mehd with total duration.

   @param[in] total_duration - total duration to be written.

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_update_mehd_atom (uint32 /*total_duration*/){return MUX_SUCCESS;};

/*!
   @brief  This is used to update the streamport.

   @detail   This is used to update the streamport to be used by mux to stream.

   @param[in] port - new streamport instance.

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_update_streamport(uint64_t /*port*/){return MUX_SUCCESS;};

/*!
   @brief  This is used to update the clipinfo.

   @detail   This is used to update the clipinfo.

   @param[in] msg_list - list of messages with clip info.

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_update_clipinfo (MUX_msg_list_type * /*msg_list*/){return MUX_SUCCESS;};

/*!
   @brief  This is used to update AVC Timing and HRD params.

   @detail   Params used to populate the Timing and
             HRD descriptor in AVC is set using this function.

   @param[in] pHRDParams - pointer to MUX_AVC_TimingHRD_params_type structure.

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_update_AVC_Timing_HRD_Params
                   (MUX_AVC_TimingHRD_params_type * /*pHRDParams*/){return MUX_FAIL;};


/*!
   @brief   Get the meta data size

   @detail  Get the meta data size

   @param[in]  none

    @return    return metadata size.
*/
virtual uint32 MUX_get_meta_data_size (void){return 0;};


/*!
   @brief  This is used to modify user data atoms.

   @detail   This is used to modify user data atoms.

   @param[in] puser_data - user data atom to be modified.
              handle   -  handle

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_modify_user_atoms (MUX_user_data_type * /*puser_data*/,
                                          void  * /*handle*/){return MUX_SUCCESS;};

#ifdef FILEMUX_WRITE_ASYNC
/*!
   @brief  This is used to flush the buffers into the file asynchronously.

   @detail   This is used to flush the buffers into the file asynchronously.

   @param[in] none

   @return    No.of bytes written
*/
virtual int FlushStream(void){return 0;};
#endif /* FILEMUX_WRITE_ASYNC  */

/*!
   @detail   Write Meta Data

   @detail   This is used to set meta data to be written to file. .

   @return    MUX_STATUS.
*/
virtual MUX_STATUS MUX_set_MetaData(FileMuxMetaDataType /*eType*/,
                                    FileMuxMetaData *){return MUX_SUCCESS;};


virtual MUX_STATUS MUX_get_Statistics(FileMuxStatistics * /*pMuxStatistics*/)
                                                     {return MUX_SUCCESS;};

virtual MUX_STATUS MUX_get_current_PTS(uint64 * /*pnPTS*/){return MUX_FAIL;};
};
#endif //__MUX_BASE_H__
