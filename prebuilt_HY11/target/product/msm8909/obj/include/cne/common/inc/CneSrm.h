#ifndef CNE_SRM_H
#define CNE_SRM_H

/**----------------------------------------------------------------------------
  @file CneSrm.h

-----------------------------------------------------------------------------*/

/*=============================================================================
    Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
============================================================================*/



/*=============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneSrm.h#12 $
  $DateTime: 2009/11/24 13:07:50 $
  $Author: mkarasek $
  $Change: 1094989 $

  when        who  what, where, why
  ----------  ---  -------------------------------------------------------
  2009-07-15  ysk  First revision.

============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "CneSrmDefs.h"
#include "EventDispatcher.h"
#include "CneDefs.h"
#include <string>
#include <cutils/properties.h>
#include <map>
#include <list>
#include "CneSupplicantWrapper.h"

using namespace std;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
class Cne;
class CneResourceInfo;
class CneBatteryInfo;
class CneWlanInfo;
class CneWwanInfo;

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/
class CneSrm : public EventDispatcher<SrmEvent>
{
public:

  CneSrm
  (
    Cne &setCne
  );

  ~CneSrm();

  static CneSrm* getInstance
  (
    void
  );

  CneRetType putResourceInfo
  (
    int resType,
    void *res
  );

  int getListResourceInfo
  (
    int numItem,
    int resType,
    void *listRes
  );

  CneRetType bringUpRAT
  (
    CneRatInfoType ratInfo,
    CneSrmRatAcqStatusCbType cbFn,
    void* cbData
  );

  CneRetType bringDownRAT
  (
    CneRatInfoType ratInfo,
    CneSrmRatAcqStatusCbType cbFn,
    void* cbData
  );

  CneRetType startScanWlan
  (
    void
  );

  CneRetType getCurrentBatteryLevel
  (
    CneSrmBatteryLevelType* batteryLevel
  );

  CneRetType updateWlanScanResults
  (
    void *res
  );

  CneRetType updateWwanDormancyStatus
  (
    int status
  );

  bool getFeatureStatus(
     CneFeatureInfoType* cneFeatureInfoCmd
  );

  void setFeatureStatus(
     CneFeatureInfoType* cneFeatureInfoCmd
  );

  list<CneWlanScanListResourceType> getWlanScanResults(void);

  static void processCommand
  (
    int fd,
    cne_cmd_enum_type cmdType,
    void *cmdData,
    void *handlerDataPtr
  );

  static void processCneCmd
  (
    CneEvent event,
    const void *event_data,
    void *cbData
  );

  /* check if a RAT is dormant
   * returns: true if dormant, otherwise false
   */
  bool isRatDormant(cne_rat_type rat);

  /* check which network is the default
   * returns: default network
   */
  cne_rat_type getDefaultNetwork();

  //Reset default network variable if none available
  void invalidateDefaultNetwork();

  CneRetType AddAppInfoList( const CneAppInfoMsgDataType *appInfo  );

  list<CneBrowserAppListInfoType> GetBrowserAppInfoList();

  const CneAppInfoMultiMapType& GetAppInfoMap();

  //Returns the number of available interfaces
  unsigned int getNumInterfacesAvailable();
  bool isIfaceSelectable(char *ifname);

  // return the current wlan info
  CneWlanResourceType getWlanInfo();

  // return the current wwan info
  CneWwanResourceType getWwanInfo();

  CneRetType UpdateBrowserAppInfoList(  const CneBrowserAppType *browserAppInfo );

  void SendGetBrowserAppInfoListReq( void );

  const char * getWifiIfaceDefName ();
  // returns whether WQE is enabled/disabled by user
  bool hasUserEnabledWqe(void);

  bool getAppInfoByName(const char *appName, AppInfoType& appInfo);

/*----------------------------------------------------------------------------
 * FUNCTION     CneSupplicantWrapper& getSupplicant
 *
 * DESCRIPTION  returns the reference to CneSupplicantWrapper object
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE a reference to CneSupplicantWrapper object
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  CneSupplicantWrapper& getSupplicant();

/*----------------------------------------------------------------------------
 * FUNCTION     updateWlanInfo
 *
 * DESCRIPTION  update wlanInfo stored in srm by calling
 *  processCneCmd with CNE_REQUEST_UPDATE_WLAN_INFO_CMD event
 *
 * DEPENDENCIES CneWlanResourceType, CneWlanInfo.cpp, EventDispatcher
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateWlanInfo(CneWlanResourceType &wlanInfo);

  private:
    static CneSrm* instancePtr;
    CneSupplicantWrapper srmSupp;
    Cne &cne;
    CneResourceInfo *resInfo;
    CneBatteryInfo *batInfo;
    CneWlanInfo *wlanInfo;
    CneWwanInfo *wwanInfo;
    CneWlanInfo *lastConnectedWlanInfo;
    CneWwanInfo *lastConnectedWwanInfo;
    // TODO
    // Switch from system properties to
    // database after initial merge
    // CneFeatureStore *fs;

    CneAppInfoMultiMapType appInfoMultiMap;
    list<CneBrowserAppListInfoType> browserAppList;
    static bool isWwanDormant;
    static cne_rat_type defaultNetwork;
    bool isDhcpRenewState( CneWlanInfoType &curr, CneWlanResourceType &prev );
    bool dhcpRenewState;
    bool bWqe;
    char wifiIfaceDefName[PROPERTY_VALUE_MAX];
    char _ssid[CNE_MAX_SSID_LEN];
    char _dnsInfo[CNE_MAX_DNS_ADDRS][CNE_MAX_IPADDR_LEN];
    bool _isLastStatusConnectedOrConnecting;
    bool _isCurrentStatusConnectedOrConnecting;

/*----------------------------------------------------------------------------
 * FUNCTION     startMonitorBssid
 *
 * DESCRIPTION  It calls startMonitorBssidChange in CneSupplicantWrapper.cpp
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
    void startMonitorBssid();
/*----------------------------------------------------------------------------
 * FUNCTION     startMonitorBssid
 *
 * DESCRIPTION  It calls stopMonitorBssidChange in CneSupplicantWrapper.cpp
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
    void stopMonitorBssid();

/*----------------------------------------------------------------------------
 * FUNCTION    sendTetheringInfoToIPA
 *
 * DESCRIPTION Send Tethering Upstream information to IPA Driver
 *
 * DEPENDENCIES
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
    void sendTetheringInfoToIPA( CneTetheringUpstreamInfo &tetheringUpstreamInfo );

};

#endif /* CNE_SRM_H */
