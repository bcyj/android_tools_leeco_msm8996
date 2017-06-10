//=============================================================================
// IEnhancedStreamSwitchManager.h
//
// COPYRIGHT 2011-2013 Qualcomm Technologies Incorporated.
// All rights reserved. Qualcomm Technologies proprietary and confidential.
//
// $Header: //depot/asic/msmshared/users/crd_dash/QSM/inc/IEnhancedStreamSwitchManager.h#38 $
// $DateTime: 2013/06/10 14:31:40 $
// $Change: 3896144 $
//=============================================================================

#ifndef I_ENHANCED_STREAM_SWITCH_MANAGER_H
#define I_ENHANCED_STREAM_SWITCH_MANAGER_H

#include "QsmTypes.h"
#include "IStreamSource.h"
#include "IDataStateProvider.h"

namespace QSM {

class IEnhancedStreamSwitchManager {
public:
   virtual ~IEnhancedStreamSwitchManager() {
   }
   ;
   /**
    * Create an instance of Enhanced Stream Switch Manager.
    *
    * param[in] pIStreamSource - the stream source that the stream manager
    * will manage
    * param[in] pIDataStateProvider - the class to provide the data state
    * (bandwidth, buffer occupancy, space etc)
    *
    * @return reference to StreamSwitchManagerInterface on success, NULL otherwise
    */
   static IEnhancedStreamSwitchManager*
         CreateInstance(IStreamSource* pIStreamSource,
                        IDataStateProvider* pIDataStateProvider);

   /**
    * Destroys the Stream Switch Manager.
    *
    */
   static void DestroyInstance(IEnhancedStreamSwitchManager* instance);

   /**
    * Instructs the Stream manger that the playback time has been
    * set to new position
    *
    * @param[in] nStartTime the new start time
    *
    * @return QsmStatus
    */
   virtual QsmStatus Play(uint64 nStartTime) = 0;

   /**
    * Instructs the Stream manger that the playback time has been
    * paused
    *
    * @return QsmStatus - QSM_STATUS_OK on success
    */
   virtual QsmStatus Pause(void) = 0;

   /**
    * Instructs the Stream manger that the playback time has been
    * resumed
    *
    * @return QsmStatus - QSM_STATUS_OK on success
    */
   virtual QsmStatus Resume(void) = 0;

   /**
    * Instructs the Stream manger that the playback has stopped,
    * clean up all the session level resources and information
    *
    * @return QsmStatus - QSM_STATUS_OK on success
    */
   virtual QsmStatus Stop(void) = 0;

   /**
    * Suspend all QSM activity and wait for RESUME to start
    * @return QsmStatus - QSM_STATUS_OK on success
    */
   virtual QsmStatus Suspend(void) = 0;

   /**
    * Notification to the stream switch manager that the select
    * representation has been done for an earlier request. It does
    * not mean that the new representation has been selected but a
    * notification that the source component has queued up the
    * switch and when the playback reaches the time specified in
    * the select representation call the switch will be done
    *
    * @param[in] array of SelectGroupRepresntation structures
    * @param[in] length of array
    * @param[in] bResult
    *           TRUE =>  all selections are switched successfully
    *           FALSE => at least one of the selections has failed to switch.
    *        GroupRepresentationSelection structure has an boolean member to
    *        indicate the groups that failed to switch to new representation
    */
   virtual void SelectRepresentationDone(GroupRepresentationSelection* c,
         uint32 nNumGroupRepresentationSelection, SelRepStatus bResult) = 0;

   /**
    * Indication to the stream manager that download has been
    * completed for a previously issued download request
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey key of the new representation
    * @param[in] nKey key of the downloadable unit
    * @param[in] bResult true indicates the download is complete
    *       else download could not be done
    *
    * @return void
    */
   enum DataUnitDownloadCompletionStatus
   {
      DOWNLOAD_SUCCESS,
      DOWNLOAD_FAILED,
      DOWNLOAD_CANCELLED,
      DOWNLOAD_MEMORY_FAILURE,
      DOWNLOAD_FAILED_DATA_NOT_AVAILABLE
   };
   virtual void DownloadDataUnitDone(uint64 nGroup, uint64 nRepresentationKey,
         uint64 nKey, DataUnitDownloadCompletionStatus status) = 0;

   /**
    * Notifies the enhanced stream switch manager that data unit
    * information for previously issued request is avaliable
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey key of the representation for
    *       which the data units belong
    * @param[in] nStartTime the start time for the available units info
    * @param[in] nDuration the duration for the available units info
    * @param[in] nNumberofUnits number of units defined in the
    *       array
    * @param[in] eStatus return status from the streamsource
    *
    * @return void
    */
   virtual void DataUnitsInfoAvaliable(uint64 nGroup,
         uint64 nRepresentationKey, uint64 nStartTime, uint64 nDuration,
         uint32 nNumberofUnits, IStreamSourceStatus eStatus) = 0;

   /**
    * Notifies the enhanced stream switch manager that new
    * representation information is available. Upon receiving this
    * notification, ESSM is expected to query it's client for new
    * group and representation info
    *
    * @return void
    */
   virtual void RepresentationInfoAvailable(void) = 0;

   /**
    * Get size in bytes needed to store relevant state information
    *
    * @return size of the state
    */
   virtual uint32 GetStateSize() const = 0;

   /**
    * Method to retrieve state information from the QSM instance into
    * provided buffer
    *
    * @param[in] ptr pointer to the buffer
    * @param[in] nBytes size of the buffer
    *
    * @return QSM_STATUS_OK on success
    */
   virtual QsmStatus RetrieveHistory(uint8* ptr, uint32 nBytes) const = 0;

   /**
    * Method to seed state information from that provided in the buffer
    *
    * @param[in] ptr pointer to the buffer
    * @param[in] nBytes size of the buffer
    *
    * @return QSM_STATUS_OK on success
    */
   virtual QsmStatus SeedHistory(uint8* ptr, uint32 nBytes) const = 0;

   /**
    * Update the start-up and re-buffer pre-roll values
    *
    * @param[in] nStartup the pre-roll value used at start of playback
    * @param[in] nRebuf the pre-roll value used during re-buffering
    *
    * @return QSM_STATUS_OK on success
    */
   virtual QsmStatus UpdateBufferPrerollValues(uint32& nStartup, uint32& nRebuf) = 0;

   enum DataUnitDownloadProgressStatus
   {
      DOWNLOAD_TOO_SLOW
   };

   /**
    * Notifies the enhanced stream switch manager that data unit
    * download is not progressing well
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey key of the representation for
    *       which the data units belong
    * @param[in] nKey hey of the data unit that is being downloaded
    * @param[in] status status code
    *
    * @return QSM_STATUS_OK on success
    */
   virtual QsmStatus DataUnitDownloadProgressInd(uint64 nGroupKey,
                                                 uint64 nRepresentationKey,
                                                 uint64 nKey,
                                                 DataUnitDownloadProgressStatus status) = 0;

   struct AdaptationSetChangeType
   {
      // Type of change requested
      enum ChangeType
      {
         ADD,
         REPLACE,
         REMOVE
      };
      ChangeType mType;

      // ADD: The adaptation set that is added
      // REMOVE: The one that is being removed
      // REPLACE: The one that is replacing the old one
      //          The adaptation set type is used to determine
      //          the one being replaced
      CGroupInfo mAsInfo;
   };

   virtual QsmStatus
   AdaptationSetChangeRequest(uint32 nTid,
                              AdaptationSetChangeType) = 0;

    /**
    * Enable/disable InitialRateEstimation feature
    *
    * @param[in] val set to true/false
    *
    */

   virtual void SetEnableInitialRateEstimation(bool val) = 0;

};

}

extern "C" void* QSMCreateInstance(QSM::IStreamSource* pIStreamSource,
                                   QSM::IDataStateProvider* pIDataStateProvider);

extern "C" void QSMDeleteInstance(QSM::IEnhancedStreamSwitchManager* pIEnhancedStreamSwitchManager);

#endif //I_ENHANCED_STREAM_SWITCH_MANAGER_H
