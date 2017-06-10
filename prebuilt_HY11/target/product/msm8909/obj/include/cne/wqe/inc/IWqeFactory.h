#ifndef WQE_FACTORY_H
#define WQE_FACTORY_H

/**----------------------------------------------------------------------------
  @file WqeFactory.h

-----------------------------------------------------------------------------*/

/*=============================================================================
    Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
============================================================================*/


/*=============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneSrm.h#12 $
  $DateTime: 2009/11/24 13:07:50 $
  $Author: mkarasek $
  $Change: 1094989 $

  when        who      what, where, why
  ----------  ---      ----------------------------------------------------
  2014-05-09  npoddar  First revision.

============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include "CneCom.h"
#include "CneTimer.h"
#include "SwimNetlinkSocket.h"
#include "SocketWrapperClient.h"
#include "CneWqeInterface.h"

class SwimInterfaceSelection;
class SwimBitrateEstimator;
class IBitrateEstimator;
class IInterfaceSelection;
class IDefaultInterfaceSelector;
class CnePolicyConfig;
class CneDefaultDnsSelector;
class SwimSocketManager;

class WqeFactory:public CneWqeInterface
{
public:

  /*----------------------------------------------------------------------------
   * Public Types
   * -------------------------------------------------------------------------*/

  /*----------------------------------------------------------------------------
   * Public Method Specifications
   * -------------------------------------------------------------------------*/

   /*----------------------------------------------------------------------------
   * FUNCTION      constructor
   *
   * DESCRIPTION   Create instance of WqeFactory class
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   WqeFactory();

   /*----------------------------------------------------------------------------
   * FUNCTION      destructor
   *
   * DESCRIPTION   deletes necessary class variables
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   virtual ~WqeFactory();

  /*----------------------------------------------------------------------------
   * FUNCTION      createFactory
   *
   * DESCRIPTION   Method to initialize required WQE classes.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   virtual void createWqe(CneCom &com, CneTimer &timer, bool isSwimEnabled);

   /*----------------------------------------------------------------------------
   * FUNCTION      getInstance
   *
   * DESCRIPTION   Return instance of WqeFactory class.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   static WqeFactory* getInstance();

   /*----------------------------------------------------------------------------
   * FUNCTION      getBitrateEstimator
   *
   * DESCRIPTION   Return instance of SwimBitrateEstimator class.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   virtual IBitrateEstimator* getBitrateEstimator();

   /*----------------------------------------------------------------------------
   * FUNCTION      getNetlinkSocket
   *
   * DESCRIPTION   Return instance of netlink socket.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   virtual SwimNetlinkSocket *getNetlinkSocket();

   /*----------------------------------------------------------------------------
    * FUNCTION      isBbdDisabled
    *
    * DESCRIPTION   Checks if BBD is enabled/disabled in policy file
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  true if disabled false otherwise
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual bool isBbdDisabled();

   /*----------------------------------------------------------------------------
    * FUNCTION      detectBrokenBackhaul
    *
    * DESCRIPTION   Proxy method which calls the actual method in
    *               SwimDefaultInterfaceSelector class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  CNE_RET_OK / CNE_RET_ERROR
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual CneRetType detectBrokenBackhaul(CneIpcReqMsg_t& req, int fd);

   /*----------------------------------------------------------------------------
    * FUNCTION      isDefaultSelectorStateWlanGood
    *
    * DESCRIPTION   Returns whether WQE State machine is in WLAN_GOOD
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  true if WLAN is the state else false
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual bool isDefaultSelectorStateWlanGood();

   /*----------------------------------------------------------------------------
    * FUNCTION      Select
    *
    * DESCRIPTION   Proxy method for the actual Select method in
    *               SwimInterfaceSelection class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  SwimNimsRetCodeType_t
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual SwimNimsRetCodeType_t Select( int rfd, SwimNimsSockAddr_t& src, SwimNimsSockAddr_t& dst,
                               int comfd, int client_fd_val, int uid, int pid);

   /*----------------------------------------------------------------------------
    * FUNCTION      verifySockExists
    *
    * DESCRIPTION   Proxy method for the actual verifySockExists method in
    *               SwimSocketManager class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  bool
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual bool verifySockExists(int uid, int pid, int fd_val, int rfd);

   /*----------------------------------------------------------------------------
    * FUNCTION      updateAppSockFd
    *
    * DESCRIPTION   Proxy method for the actual updateAppSockFd method in
    *               SwimSocketManager class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  bool
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual void updateAppSockFd(int uid, int pid, int fd_val, int rfd);

   /*----------------------------------------------------------------------------
    * FUNCTION      handleHangup
    *
    * DESCRIPTION   Proxy method for the actual handleHangup method in
    *               SwimSocketManager class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  bool
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual void handleHangup(int comfd);

   /*----------------------------------------------------------------------------
    * FUNCTION      handleClose
    *
    * DESCRIPTION   Proxy method for the actual handleClose method in
    *               SwimSocketManager class
    *
    * DEPENDENCIES  Logging
    *
    * RETURN VALUE  bool
    *
    * SIDE EFFECTS  None
    *--------------------------------------------------------------------------*/
   virtual void handleClose(int uid, int pid, int comfd, int client_fd_val);

private:

  /*----------------------------------------------------------------------------
   * Private Attributes
   * -------------------------------------------------------------------------*/
   SwimInterfaceSelection *ifaceSel;

   static WqeFactory *instPtr;

   IBitrateEstimator *bee;

   SwimNetlinkSocket *netlink;

   CnePolicyConfig *policyConfig;

   CneDefaultDnsSelector *dnsSel;

   bool isFactoryInitialized;

   SwimSocketManager *smgr;
};

#endif /* _WqeFactory_h_ */
