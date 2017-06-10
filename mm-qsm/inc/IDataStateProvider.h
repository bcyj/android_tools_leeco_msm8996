//=============================================================================
// IDataStateProvider.h
//
// COPYRIGHT 2011-2013 Qualcomm Technologies Incorporated.
// All rights reserved. Qualcomm Technologies proprietary and confidential.
//
// $Header: //depot/asic/msmshared/users/crd_dash/QSM/inc/IDataStateProvider.h#26 $
// $DateTime: 2013/12/02 15:37:24 $
// $Change: 4884753 $
//=============================================================================

#ifndef I_DATA_STATE_PROVIDER_H
#define I_DATA_STATE_PROVIDER_H

#include "QsmTypes.h"

namespace QSM {

typedef enum {
   DSP_STATUS_OK, DSP_STATUS_FAILURE
} IDataStateProviderStatus;


class IDataStateProvider {
public:
   virtual ~IDataStateProvider() {
   }
   ;

   /**
    * Returns the cumulative download statistics needed by DASH 2.0 filters
    *
    * @param[out] nTotalDownloadTime total amount of wall time (in msec) spent
    *     downloading data
    * @param[out] nTotalBytesDownloaded total number of bytes received
    */
   virtual IDataStateProviderStatus
       GetCumulativeDownloadStats(uint64& nTotalDownloadTime,
                                  uint64& nTotalBytesDownloaded) = 0;

   /**
    * Get playback statistics for a particular group such as the buffer occupancy
    * and playback time
    *
    * @param [in] nGroupKey group for which stats are generated
    *             a value of MAX_UINT64 indicates video group
    * @param [out] nFullyDownloadedOccupancy occupancy information (in milliseconds)
    *                                        based on fully downloaded data units for
    *                                        the particular group
    * @param [out] nPartiallyDownloadedOccupancy occupancy information (in milliseconds)
    *                                            based on partially downloaded data units
    *                                            for the particular group
    * @param [out] nTotalOccupancy occupancy information (in milliseconds) that includes
    *                              un-filled, partially filled and completed data units for
    *                              the particular group.
    * @param [out] nPlaybackTime   current playback time for this group
    * @param [out] nForwardMediaAvailability amount of presentation time (in milliseconds)QSM is
    *                                        allowed to download beyond the current playback time
    *
    * @return status indicating success or failure
    */
    virtual IDataStateProviderStatus GetGroupPlaybackStats(uint64 nGroupKey,
                                                           uint32& nFullyDownloadedOccupancy,
                                                           uint32& nPartiallyDownloadedOccupancy,
                                                           uint32& nTotalOccupancy,
                                                           uint64& nPlaybackTime,
                                                           uint32& nForwardMediaAvailability) = 0;
};

}

#endif //I_DATA_STATE_PROVIDER_H
