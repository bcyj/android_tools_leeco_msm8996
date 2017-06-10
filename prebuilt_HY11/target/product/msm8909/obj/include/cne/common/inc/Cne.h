#ifndef CNE_H
#define CNE_H

/**----------------------------------------------------------------------------
  @file Cne.h

  This file provides the CNE interface/Service for the clients.
  It provides the info about all the commands that CNE can handle
  and all the events that it can send to the clients.

-----------------------------------------------------------------------------*/

/**----------------------------------------------------------------------------
  @mainpage Connectivity Engine Interface.
    This documentation defines the interface for the Connectivity Engine(CNE).

    Connectivity Engine provides differenct services for its clients.

    Given the role the client wants to play, it will suggest the best network
    that it can use for the best user experience.

    The suggestion are based of carrier profiles and user requirments.

    Clients can also specify its QOS requirments.

    Curently in this release clients can specify only band width requirements.

    Clients will get notified on loss of connectivity on their current rat.

    Clients can get notified if a better rat is availble based of its
    requirments.

-----------------------------------------------------------------------------*/

/*==============================================================================
  FILE:         Cne.h

  OVERVIEW:     Connectivity Engine

  DEPENDENCIES: Logging, sockets

                Copyright (c) 2011, 2012, 2013 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ---------------------------------------------------------
  04-16-2012  mtony   Removed circular dependency btwn Cne and CneCom
=============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/
#include <binder/Parcel.h>
#include <time.h>

#include "CneTimer.h"
#include "CneCom.h"
#include "CneSrm.h"
#include "CneDefs.h"
#include "EventDispatcher.h"
#include "CneParcel.h"
#include "CneUtils.h"
#include "CneMsg.h"
#include "CnePermissions.h"

#define PROC_MEMORY_FILE "/proc/meminfo"
#define MEMORY_SIZE_MIN (512 * 1024 * 1024)

/*----------------------------------------------------------------------------
 * Class Declarations and Documentation
 * -------------------------------------------------------------------------*/

class Cne : public ICneTimerMonitor, public EventDispatcher<CneEvent> {
public:
  /*----------------------------------------------------------------------------
   * FUNCTION      Constructor
   *
   * DESCRIPTION   init CneSrm, CneCde, SwimNims
   *
   * DEPENDENCIES  CneTimer, CneCom, CneSrm, CneCde, SwimNims
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  Cne();

  /*----------------------------------------------------------------------------
   * FUNCTION      Destructor
   *
   * DESCRIPTION   -
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  virtual ~Cne();

  /*----------------------------------------------------------------------------
   * FUNCTION      run
   *
   * DESCRIPTION   starts main event loop for Cne
   *
   * DEPENDENCIES  CneMsg, fcntl, sockets
   *
   * RETURN VALUE  this method does not return
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  void run();

  /*----------------------------------------------------------------------------
   * FUNCTION      notifyDelayChange
   *
   * DESCRIPTION   called by CneTimer whenever the time to next callback
   *               is changed unexpectedly; required by ICneTimerMonitor
   *
   * DEPENDENCIES  CneTimer, CneCom
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  epoll is woken up, and its delay is recalculated
   *--------------------------------------------------------------------------*/
  void notifyDelayChange();

  /*----------------------------------------------------------------------------
   * FUNCTION      getTime
   *
   * DESCRIPTION   makes a system call to clock_gettime and returns the value
   *               as a double
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  static double getClockTime(clockid_t clockId = CLOCK_REALTIME);

  /*----------------------------------------------------------------------------
   * FUNCTION      doubleToTimespec
   *
   * DESCRIPTION   converts a double into a timespec struct
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  static void doubleToTimespec(double t, timespec& ts);

  /*----------------------------------------------------------------------------
   * FUNCTION      getPermissions
   *
   * DESCRIPTION   provides access to cne permissions object
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
   CnePermissions getPermissions();

  /*----------------------------------------------------------------------------
   * FUNCTION      getCom
   *
   * DESCRIPTION   provides access to cne com object
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
   CneCom& getCom();

  /*----------------------------------------------------------------------------
   * FUNCTION      getTmer
   *
   * DESCRIPTION   provides access to cne timer object
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
   CneTimer& getTimer();

  /*----------------------------------------------------------------------------
   * FUNCTION      getSrm
   *
   * DESCRIPTION   provides access to cne srm object
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
   CneSrm& getSrm();

private:
  CneTimer timer;
  CneCom com;
  CneSrm srm;
  CnePermissions cnePerm;
  static const unsigned int MAX_READ_SIZE = (8 * 1024);
  static const char SOCKET_NAME_CNE[];

  static int cneSocketFd; // "cnd" socket
  static int count; // number of commands processed

  // Handles writes to the cnd socket
  static void processCneEvent(int fd, void *pCne);
  // Helper method for processCneEvent
  static void parcelAndDispatch(int fd, Cne *cne, void *record, size_t recordLen);
  // Handles client disconnects on the cnd socket
  static void processCneCloseEvent(int fd, void* pCneSocket);
  // Handles incoming connections on the cnd socket
  static void listenSocketCallback(int listeningFd, void *data);
};

#endif /* CNE_H */
