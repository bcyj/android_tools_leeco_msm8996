/*==============================================================================
*        @file SessionManager.cpp
*
*  @par DESCRIPTION:
*        SessionManager class.
*
*
*  Copyright (c) 2012 -2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <utils/Log.h>
#ifndef WFD_ICS
#include <common_log.h>
#endif
#include "SessionManager.h"
#include "wfd_netutils.h"
#include "wifidisplay.h"
#include "Device.h"
#include "WFDSession.h"
#include "RTSPSession.h"
#include "MMAdaptor.h"
#include "UIBCAdaptor.h"
#include "MMCapability.h"


SessionManager* SessionManager::uniqueInstance = NULL;

SessionManager* SessionManager::Instance() {
	if (uniqueInstance == NULL) {
		uniqueInstance = new SessionManager();
	}
	return uniqueInstance;
}

void SessionManager::DeleteInstance() {
  if (uniqueInstance) {
     delete uniqueInstance;
     uniqueInstance = NULL;
  }
}

SessionManager::SessionManager() :
    pMyDevice(NULL) {
    rtspSessionId = -1;
    serviceState = DISABLED;
}

SessionManager::~SessionManager() {

    if (pMyDevice) {
      delete pMyDevice;
    }
    pMyDevice = NULL;
    vector<WFDSession*>::iterator it;
    for (it=vecWFDSessions.begin(); it!=vecWFDSessions.end(); it++) {
        if (*it != NULL)
          delete(*it);
    }
}


/**
 * Function to enable WFD service.  It queries the
 * local wifi info from WLAN module, then configures myDevice.
 * @param devType
 */
void SessionManager::enableWfd(Device *thisDevice) {
	if (pMyDevice) {
		delete pMyDevice;
		pMyDevice = NULL;
	}

    pMyDevice = thisDevice;
    //Setting the UIBC Device Type
    UIBCAdaptor::setDeviceType(thisDevice->getDeviceType());
    UIBCAdaptor::createUibcInterface();
    // configure local device info
    MMAdaptor::updateLocalMMCapability(pMyDevice->pMMCapability, thisDevice->getDeviceType());
    UIBCAdaptor::updateLocalUIBCCapability(pMyDevice->pMMCapability->pUibcCapability);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Local MM capability dump:");
    pMyDevice->pMMCapability->dump();

    serviceStateTransition(ENABLED);
}



/**
 * Function to disable WFD service
 */
void SessionManager::disableWfd() {

    UIBCAdaptor::destroyUibcInterface();
    delete pMyDevice;
    pMyDevice = NULL;
    serviceStateTransition(DISABLED);
}

/**
 * Function to transition the serviceState between ENABLED and
 * DISABLED
 * @param state
 */
void SessionManager::serviceStateTransition(ServiceState newState) {
    if (serviceState != newState) {
        serviceState = newState;

        if (serviceState == ENABLED) {
            eventServiceEnabled();
        } else {
            eventServiceDisabled();
        }
    }
}

/**
 * Function to start a WFD session
 * @param peer Device info
 */
bool SessionManager::startWfdSession(WfdDevice *pPeerDevice) {
    // update local IP if it's not updated yet
    if (pMyDevice->ipAddr.empty()) {
        char ip[20];
        int ret = getLocalIpAddress(ip, sizeof(ip));
        if (ret) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to get local IP address, aborting startWfdSession");
            //If IP is not available for some reason here then don't bail out
            //Local IP can be retrieved at a later stage and updated
        }
        else {
            pMyDevice->ipAddr = ip;
            MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Local IP updated: %s", pMyDevice->ipAddr.c_str());
        }
    }
    //string peerMacStr = string(pPeerMacAddr);
    WFDSession* pSession = new WFDSession(pMyDevice->getDeviceType(), pPeerDevice, STREAMING);
    vecWFDSessions.insert(vecWFDSessions.end(), pSession);

    // start the WFD session state machine
    if(false == pSession->start()) {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Start WFD failed");
      return false;
    }
    return true;
}

void SessionManager::stopWfdSession(int rtspId) {

  vector<WFDSession*>::iterator it;
  int cnt = 0;
  MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"StopWfdSession");
  for (it= vecWFDSessions.begin(); it!= vecWFDSessions.end(); it++,cnt++) {
      WFDSession* wfdSession = *it;
      if (wfdSession != NULL ){
         RTSPSession* rtspSession = wfdSession->getRtspSession();
        if (rtspSession && rtspSession->rtspSessionId == rtspId) {
          //Destroying session based on RTSP session ID
            delete wfdSession;
            vecWFDSessions.erase(vecWFDSessions.begin() + cnt);
            break;
        }
      }
  }
}

void SessionManager::setUserCapability(int capType, void* value) {
  MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Setting user capability");
  if (value == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Input value is NULL ");
    return;
  }
  switch (capType)
  {
  case    UIBC_Generic:
    if (strcasecmp((const char*)value,"enable") != 0) {
      userCapabilities[UIBC_Generic] = ENABLE;
    }
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[UIBC_Generic] = DISABLE;
    break;
  case    UIBC_INPUT:
    if (strcasecmp((const char*)value,"Keyboard") != 0)
      userCapabilities[UIBC_INPUT] = ENABLE;
    else if (strcasecmp((const char*)value,"Mouse") !=0 )
      userCapabilities[UIBC_INPUT] = DISABLE;
    break;
  case    HDCP:
    if (strcasecmp((const char*)value,"enable") != 0)
      userCapabilities[HDCP] = ENABLE;
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[HDCP] = DISABLE;
    break;
  case    FRAME_SKIPPING:
    if (strcasecmp((const char*)value,"enable") != 0)
      userCapabilities[FRAME_SKIPPING] = ENABLE;
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[FRAME_SKIPPING] = DISABLE;
    break;
  case    STANDBY_CAP:
    if (strcasecmp((const char*)value,"enable") != 0)
      userCapabilities[STANDBY_CAP] = ENABLE;
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[STANDBY_CAP] = DISABLE;
    break;
  case    INPUT_VIDEO:
    if (strcasecmp((const char*)value,"Protected") != 0)
      userCapabilities[INPUT_VIDEO] = ENABLE;
    else if (strcasecmp((const char*)value,"Unprotected") !=0 )
      userCapabilities[INPUT_VIDEO] = DISABLE;
    break;
  case    VIDEO_RECOVERY:
    if (strcasecmp((const char*)value,"enable") != 0)
      userCapabilities[VIDEO_RECOVERY] = ENABLE;
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[VIDEO_RECOVERY] = DISABLE;
    break;
  case    PREFFERED_DISPLAY:
    if (strcasecmp((const char*)value,"enable") != 0)
      userCapabilities[PREFFERED_DISPLAY] = ENABLE;
    else if (strcasecmp((const char*)value,"disable") !=0 )
      userCapabilities[PREFFERED_DISPLAY] = DISABLE;
    break;
  default :
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Unknown/Unsupported capability");
    break;
  }


}

void SessionManager::modifyUserCapability(int capType, MMCapability* localCap, CapabilityStatus status) {
  MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Modifying user capability");

  switch (capType)
  {
  case    UIBC_Generic:
    if (status  == ENABLE) {
      localCap->pUibcCapability->config.category |=GENERIC ;
    }
    else if (status == DISABLE)
      localCap->pUibcCapability->config.category &= ~GENERIC ;
    break;
  case    UIBC_INPUT:
    if (status  == ENABLE)
      localCap->pUibcCapability->config.generic_input_type |= MOUSE;
    else if (status == DISABLE)
      localCap->pUibcCapability->config.generic_input_type |= KEYBOARD;
    break;
  case    HDCP:
    // TODO
    break;
  case    FRAME_SKIPPING:
    // TODO
    break;
  case    STANDBY_CAP:
    if (status == ENABLE)
      localCap->pCapability->standby_resume_support = true;
    else if (status == DISABLE )
      localCap->pCapability->standby_resume_support = false;
    break;
  case    INPUT_VIDEO:
    // TODO
    break;
  case    VIDEO_RECOVERY:
    // TODO
    break;
  case    PREFFERED_DISPLAY:
    //TODO
    break;
  default :
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Unknown/Unsupported capability");
    break;
  }

}

