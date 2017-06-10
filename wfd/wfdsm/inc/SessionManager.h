/*==============================================================================
*        @file SessionManager.h
*
*  @par DESCRIPTION:
*        SessionManager class.
*
*
*  Copyright (c) 2012 - 2013 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _SESSIONMANAGER_H
#define _SESSIONMANAGER_H

#include "WFDDefs.h"
#include "Device.h"
#include "WFDSession.h"
#include <vector>
using namespace std;


/* Forward Declarations */
class Device;
class WFDSession;


/**----------------------------------------------------------------------------
   SessionManager class
-------------------------------------------------------------------------------
*/

class SessionManager
{
private:
	static SessionManager *uniqueInstance;
    ServiceState serviceState;

    SessionManager();
    ~SessionManager();
    void serviceStateTransition(ServiceState);


public:
    static SessionManager *Instance();
    static void DeleteInstance();
    CapabilityStatus userCapabilities[MAX_CAPABILITY];
    Device* pMyDevice;
    vector<WFDSession*> vecWFDSessions;
    int rtspSessionId;

    void enableWfd(Device *thisDevice);
    void disableWfd();

    bool startWfdSession(WfdDevice* peerDevice);
    void stopWfdSession(int rtspId);
    int  getRTSPSessionId(){ return rtspSessionId;}
    void setUserCapability(int capType, void*);
    void modifyUserCapability(int capType, MMCapability* , CapabilityStatus);

};


#endif /*_SESSIONMANAGER_H*/
