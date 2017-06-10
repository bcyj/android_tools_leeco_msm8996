/*==============================================================================
*        @file SessionState.h
*
*  @par DESCRIPTION:
*        WFDSession class.
*
*
*  Copyright (c) 2012 -2013 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _WFDSESSION_H
#define _WFDSESSION_H

#include "WFDDefs.h"


/* Forward Declarations */
class Device;
class RTSPSession;


typedef enum SessionState {
    SESSION_STOPPED,
    SESSION_NEGOTIATING,
    SESSION_NEGOTIATED,
    SESSION_ESTABLISHING,
    SESSION_ESTABLISHED,
    SESSION_STANDBY,
    SESSION_STARTING
} SessionState;

typedef enum SessionType {
    STREAMING,
    COUPLING
} SessionType;


/**----------------------------------------------------------------------------
   WFDSession class
-------------------------------------------------------------------------------
*/

class WFDSession
{
private:
    SessionState sessionState;
    SessionType  sessionType;

    Device       *pPeerHandle;
    RTSPSession  *pRtspSession;

    void sessionStateTransition(SessionState, bool notify = true);

public:
    WFDSession(DeviceType, WfdDevice*, SessionType);
    ~WFDSession();

    bool start();
    void updateSessionState(bool notify = true);

    RTSPSession* getRtspSession();

};




#endif /*_WFDSESSION_H*/
