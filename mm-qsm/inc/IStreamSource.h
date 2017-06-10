//=============================================================================
// IStreamSource.h
//
// COPYRIGHT 2011-2013 Qualcomm Technologies Incorporated.
// All rights reserved. Qualcomm Technologies proprietary and confidential.
//
// $Header: //depot/asic/msmshared/users/crd_dash/QSM/inc/IStreamSource.h#56 $
// $DateTime: 2014/03/13 18:08:10 $
// $Change: 5460255 $
//=============================================================================

#ifndef I_STREAM_SOURCE_H
#define I_STREAM_SOURCE_H

#include "QsmTypes.h"

namespace QSM {

typedef enum {
   SS_STATUS_OK,
   SS_STATUS_INSUFF_BUFF,
   SS_STATUS_FAILURE,
   SS_STATUS_NO_MORE_DATA,
   SS_STATUS_TIME_SHIFTED_DATA,
   SS_STATUS_INVALID_REPRESENTATION,
   SS_STATUS_DATA_NOT_AVAILABLE
} IStreamSourceStatus;

/**
 * Auxiliary structure to provide the best representation for a given group
 * that can be played starting from the given nKey.
 * The nKey time needs be greater than the current playback time
 *
 * nGroupKey             key of the group for which the selection is being made
 * nRepresentationKey    key of the new representation to be played
 * nKey                  key of the switch unit on which the switch must occur
 * nEndTime              indicates the point in time when the
 *                       switch to this representation has to
 *                       occur even in case if a RAP was not
 *                       found
 *
 * bSelectionSuccessful  used to respond to QSM selection status
 * for this group
 *                       TRUE =>  success switching for this group
 *                                   (overall selection may still fail)
 *                       FALSE => failure to switch for this group
 *                                   (overall selection must fail)
 */
struct GroupRepresentationSelection {
   uint64 nGroupKey;
   uint64 nRepresentationKey;
   uint64 nKey;
   int64  nEndTime;
   bool bSelectionSuccessful;
};

/**
 * Streaming Source Info Interface
 *  Defines interface for the stream manager to get information
 *  from source component
 */
class IStreamSource {
public:
   virtual ~IStreamSource() {
   }
   ;
   /**
    * Get source capabilities
    *
    * @param[out] info
    *
    * @return IStreamSourceStatus
    */
   virtual IStreamSourceStatus GetStreamSourceCapabilities(
         QSM::StreamSourceCapabilitiesInfo& pStreamSourceCapabilitiesInfo) = 0;

   virtual IStreamSourceStatus GetNumberOfBinningThresholdsForSwitchableGrps(
      uint32& nNumThresholds) = 0;

   /**
    * Gets the list of groups available for the session. Stream
    * Switch manager needs to select at least one representation
    * from each group. If nSizeOfGroupInfo is less than the number
    * of available groups, method populates the array with
    * nSizeOfGroupInfo elements and returns SS_STATUS_INSUFF_BUFF
    *
    * @param[in] pGroupInfo populated information on success
    * @param[in] nSizeOfGroupInfo the available elements in the
    *       group info array
    * @param[out] nNumGroupInfo the number of elements populated in
    *       the group info array
    *
    * @return SS_STATUS_OK if group info is populated with all
    *         groups or SS_STATUS_INSUFF_BUFF if provided buffer is
    *         of insufficient size or specific error code
    */
   virtual IStreamSourceStatus GetGroupInfo(QSM::CGroupInfo* pGroupInfo,
         uint32 nSizeOfGroupInfo, uint32 &nNumGroupInfo) = 0;

   /**
    * Gets the list of representations available for the group.
    * If nSizeOfRepresentationInfo is less than the number of
    * available representations, method populates the array with
    * nSizeOfRepresentationInfo elements and returns
    * SS_STATUS_INSUFF_BUFF
    *
    * @param[in] nGroupKey the group key for which the
    *       representation is being fetched
    * @param[in] pRepresentationInfo populated information on
    *       success
    * @param[in] nSizeOfRepresentationInfo the available elements
    *       in the representation info array
    * @param[out] nNumRepresentationInfo the number of elements
    *       populated in the representation info array
    *
    * @return SS_STATUS_OK if representation info is populated with
    *         all groups or SS_STATUS_INSUFF_BUFF if provided
    *         buffer is of insufficient size or specific error code
    */
   virtual IStreamSourceStatus GetRepresentationInfo(uint64 nGroupKey,
         QSM::CRepresentationInfo* pRepresentationInfo,
         uint32 nSizeOfRepresentationInfo,
         uint32 &nNumRepresentationInfo) = 0;

   /**
    * Sets best representation that can be played across all groups
    * starting from the given nKey.
    *
    * @param[in] array of SelectGroupRepresntation structures
    * @param[in] length of array
    *
    * @return IStreamSourceStatus SS_STATUS_OK on success
    *        Failure status to be returned if at least one of the groups
    *        failed to switch to the new representation
    *        GroupRepresentationSelection structure has an boolean member to
    *        indicate the groups that failed to switch to the new representation
    *
    */
   virtual IStreamSourceStatus SelectRepresentation(
         GroupRepresentationSelection* pSelection,
         uint32 nNumGroupRepresentationSelection)=0;

   /**
    * Requests the details for data units. A data unit is the most
    * optimal block of data from the source component perspective
    * that can be managed and downloaded by the source component.
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey representation for which data
    *       units details need to be downloaded
    * @param[in] nStartTime the time from which the information
    *       needs to be downloaded
    * @param[in] nDuration the duration from the start time for
    *       which the data unit details need to fetched
    *
    * @return IStreamSourceStatus   status
    */
   virtual IStreamSourceStatus RequestNumberDataUnitsInfo(uint64 nGroupKey,
         uint64 nRepresentationKey, uint64 nStartTime, uint64 nDuration) = 0;

   /**
    * Provides the buffer for data units.
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey representation for which data
    *       units details need to be downloaded.
    *       NO_REPRESENTATION_SELECTED will indicate that the
    *       representation for this group is not giong to be picked,
    *       meaning that Source component should stop the playback
    *       of this group
    * @param[in] nStartTime the time from which the information
    *       needs to be downloaded
    * @param[in] nDuration the duration from the start time for
    *       which the data unit details need to fetched
    * @param[in/out] pDataUnitInfo empty array to fill with
    *       CDataUnitInfo
    * @param[in] nElements - input size of array
    * @param[out] nFilledElements number of filled units
    * @return IStreamSourceStatus status
    */
   virtual IStreamSourceStatus ReadDataUnitsInfo(uint64 nGroupKey,
         uint64 nRepresentationKey, uint64 nStartTime, uint64 nDuration,
         QSM::CDataUnitInfo* pDataUnitInfo, uint32 nElements,
         uint32& nFilledElements) = 0;

   /**
    * Request the component to download a data unit
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey representation for data unit
    * @param[in] nKey key of the data unit
    *
    * @return IStreamSourceStatus status
    */
   virtual IStreamSourceStatus RequestDownloadDataUnit(uint64 nGroupKey,
         uint64 nRepresentationKey, uint64 nKey) = 0;

   /**
    * Cancel the download of a requested data unit
    *
    * @param[in] nGroupKey key of the group for the representation
    * @param[in] nRepresentationKey representation for data unit
    * @param[in] nKey key of the data unit
    *
    * @return IStreamSourceStatus   true on success else false
    */
   virtual IStreamSourceStatus CancelDownloadDataUnit(uint64 nGroupKey,
         uint64 nRepresentationKey, uint64 nKey) = 0;


   // Use current playback time time as the start time to use for
   // obtaining data download information
   static const uint64 USE_PLAYBACK_TIME_AS_START_TIME = MAX_UINT64;

   /**
    * Given a time t, obtain data unit download information for all groups
    * and all representations within that group, that have either been
    * downloaded or curreing being downloaded after t
    *
    * @note: This API is needed for DASH 2.0
    *
    * @param[inout] pDownloadInfo structure to store download information
    * @param[in] nSize maximum size of above structure
    * @param[out] nFilled the number of data units for which download information
    *     has been filled. If @param nSize is set to 0 or @param pDownloadInfo is
    *     insufficient to hold all the download information, this value provides the
    *     the total number of data units for which download information is available
    * @param[in] nStartTime start time from which download information needs to be
    *     populated
    * @return SS_STATUS_OK if download info is populated with all
    *         groups or SS_STATUS_INSUFF_BUFF if provided buffer is
    *         of insufficient size or specific error code
    */
    virtual IStreamSourceStatus
        GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo* pDownloadInfo,
                                uint32 nSize,
                                uint32& nFilled,
                                uint64 nStartTime = USE_PLAYBACK_TIME_AS_START_TIME) = 0;
    enum CmdType
    {
       PLAY,
       PAUSE,
       STOP,
       RESUME,
       SUSPEND
    };
    enum CmdStatus
    {
       SUCCESS,
       FAILED,
    };

    /**
     * Provide status information for the commands issued by the source
     * @param nTs time when the status was generated
     * @param cmd the command for which status is provided
     * @param status the status information for the command
     */
    virtual void CommandStatus(uint64 nTs, CmdType cmd, CmdStatus status) = 0;

    static const uint64 ALL_GROUPS = MAX_UINT64;
    /**
     * Provide indication to the source that Qsm is done making requests
     * both data and meta-data for a particular group
     * @param nGrp - group for which playback has completed
     */
    virtual void GroupRequestsCompleted(uint64 nGrp = ALL_GROUPS) = 0;

    /**
     * Provide configuration parameters required by QSM
     * @param mConfigParams QSM configuration parameters
     * @return SS_STATUS_OK if successful, an error code otherwise.
     */
    virtual IStreamSourceStatus GetQsmConfigParams(QSM::QsmConfigParams& mConfigParams) = 0;

    /**
     * Get the current playback position and occupancy across all periods for a particular group
     * @param nPlaybackTime global playback position across all groups
     * @param nOccupancy global occupancy position across all groups
     * @return SS_STATUS_OK  if successful, an error code otherwise.
     */
    virtual IStreamSourceStatus GetGlobalPlaybackStats(uint64& nPlaybackTime, uint64& nOccupancy) = 0;

    /**
     * Get current playback time within a period for a particular group
     * @param nGrpKEy key for a group
     * @param nPlaybackTime playback position for that group
     * @return SS_STATUS_OK  if successful, an error code otherwise.
     */
    virtual IStreamSourceStatus GetPlaybackTime(uint64 nGrpKey, uint64& nPlaybackTime) = 0;

    /**
     * Continue the download of a requested data unit
     * that the Source may have indicated that the data download
     * is not progressing well, etc. instead of cancelling
     *
     * @param[in] nGroupKey key of the group for the representation
     * @param[in] nRepresentationKey representation for data unit
     * @param[in] nKey key of the data unit
     *
     * @return IStreamSourceStatus   true on success else false
     */
    virtual IStreamSourceStatus ContinueDownloadDataUnit(uint64 nGroupKey,
        uint64 nRepresentationKey, uint64 nKey) = 0;

    enum AdaptationSetChangeStatus
    {
      ADAPTATION_SET_CHANGE_REQ_SUCCESS,
      ADAPTATION_SET_CHANGE_REQ_MALFORMED,
      ADAPTATION_SET_CHANGE_IN_PROGRESS,
      ADAPTATION_SET_CHANGE_REQ_FAILED
    };

    /**
     * Response for adaptation set change request issued to QSM
     *
     * @param[out] nTid Transaction id of the request
     * @param[out] s status of the change request
     * @return IStreamSourceStatus   true on success else false
     */
    virtual IStreamSourceStatus AdaptationSetChangeResponse(uint32 nTid,
                                                            AdaptationSetChangeStatus status) = 0;
};

}

#endif //I_STREAM_SOURCE_H
