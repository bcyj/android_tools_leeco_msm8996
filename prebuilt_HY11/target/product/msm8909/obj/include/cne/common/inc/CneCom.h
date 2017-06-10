#ifndef CNE_COM_H
#define CNE_COM_H

/*==============================================================================
  FILE:         CneCom.h

  OVERVIEW:     Manages communication with file descriptors for CnE

  DEPENDENCIES: Logging

                Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
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

#include <map>
#include <set>
#include <binder/Parcel.h>
#include <sys/epoll.h>

#include "CneParcel.h"
#include "CneUtils.h"
#include "CneMsg.h"
#include "CneTimer.h"

/*------------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ---------------------------------------------------------------------------*/
#undef SUB_TYPE
#define SUB_TYPE CNE_MSG_SUBTYPE_QCNEA_CORE_COM

/*------------------------------------------------------------------------------
 * CLASS         CneCom
 *
 * DESCRIPTION   Handle communications for CnE
 *----------------------------------------------------------------------------*/
class CneCom {

public:

  // signature of callback methods
  typedef void (*ComEventCallback)(int fd, void *data); // fd event
  typedef void (*ComCloseCallback)(int fd, void *data); // fd close event

  /*----------------------------------------------------------------------------
   * FUNCTION      Constructor
   *
   * DESCRIPTION   creates an epoll instance
   *
   * DEPENDENCIES  epoll, eventfd
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  CneCom();

  /*----------------------------------------------------------------------------
   * FUNCTION      processEvents
   *
   * DESCRIPTION   process any ready epoll events, waits up to a maximum of
   *               'waitTime' milliseconds. a wait time of -1 will cause this
   *               method to wait indefinitely.
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  any events will have their associated callback called
   *--------------------------------------------------------------------------*/
  void processEvents(int waitTime, CneTimer &timer);

  /*----------------------------------------------------------------------------
   * FUNCTION      addComEventHandler
   *
   * DESCRIPTION   associate a callback with an epoll event on a file
   *               descriptor; each file descriptor can have only one callback
   *               params:
   *                 fd       - file descriptor to associate with the callback
   *                 callback - pointer to callback method
   *                 data     - additional data to be passed back to callback
   *                 close    - pointer to callback method to be called
   *                            when the fd closes
   *                 events   - epoll events that will trigger the callback,
   *                            will default to EPOLLIN | EPOLLHUP
   *                            but can be overridden
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  true if the handler was added, otherwise false
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  bool addComEventHandler(int fd, ComEventCallback callback, void *data,
      ComCloseCallback close = NULL, int events = EPOLLIN | EPOLLHUP);

  /*----------------------------------------------------------------------------
   * FUNCTION      removeComEventHandler
   *
   * DESCRIPTION   disassociate a callback with an epoll event on a file
   *               descriptor, if the file descriptor does not have an
   *               associated callback no action is taken
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void removeComEventHandler(int fd);

  /*----------------------------------------------------------------------------
   * FUNCTION      wake
   *
   * DESCRIPTION   stop waiting for events to be processed
   *
   * DEPENDENCIES  epoll, eventfd
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void wake();

  /*----------------------------------------------------------------------------
   * FUNCTION      isApprovedFd
   *
   * DESCRIPTION   check to see if the given file descriptor is on the list of
   *               FD's approved for I/O.
   *
   * DEPENDENCIES
   *
   * RETURN VALUE  true if approved, otherwise false
   *
   * SIDE EFFECTS
   *--------------------------------------------------------------------------*/
  static bool isApprovedFd(int fd);

  /*----------------------------------------------------------------------------
   * FUNCTION      sendUnsolicitedMsg
   *
   * DESCRIPTION   send a message to a file descriptor
   *
   * DEPENDENCIES  CneParcel must know how to parcel the msgType, uses send()
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  the message will be parceled and sent to the target
                   file descriptor
   *--------------------------------------------------------------------------*/
  template <class T>
  static void sendUnsolicitedMsg(int targetFd, cne_msg_enum_type msgType, T const& data) {
    CNE_MSG_VERBOSE("sending unsolicited message. fd:%d type:%s (%d)",
        targetFd, CneUtils::getCneMsgStr(msgType), msgType);
    android::Parcel p;
    p.writeInt32(UNSOLICITED_MESSAGE);
    p.writeInt32(msgType);
    CneParcel::parcel(data, p);
    sendMessage(targetFd, p.data(), p.dataSize());
  }

  /*----------------------------------------------------------------------------
   * FUNCTION      sendUnsolicitedMsg
   *
   * DESCRIPTION   send an empty message to a file descriptor
   *
   * DEPENDENCIES  send()
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  the message (without payload) will be sent to the target
                   file descriptor
   *--------------------------------------------------------------------------*/
  static void sendUnsolicitedMsg(int targetFd, cne_msg_enum_type msgType);

  /*----------------------------------------------------------------------------
   * FUNCTION      sendUnsolicitedMsg
   *
   * DESCRIPTION   send a raw message to a file descriptor
   *
   * DEPENDENCIES  send()
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  the message will be sent to the target file descriptor
   *--------------------------------------------------------------------------*/
  static void sendUnsolicitedMsg(int targetFd, cne_msg_enum_type msgType, int dataLen, void *data);

  /*----------------------------------------------------------------------------
   * FUNCTION      sendAccessDenied
   *
   * DESCRIPTION   notify an FD that permission to access CNE is denied
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  the message will be sent to the target file descriptor
   *--------------------------------------------------------------------------*/
  static void sendAccessDenied(int fd);

  /*----------------------------------------------------------------------------
   * FUNCTION      sendAccessAllowed
   *
   * DESCRIPTION   notify an FD that permission to access CNE is allowed
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  the message will be sent to the target file descriptor
   *--------------------------------------------------------------------------*/
  static void sendAccessAllowed(int fd);

  /*----------------------------------------------------------------------------
   * FUNCTION      getClientFd
   *
   * DESCRIPTION   get the CNEJ file descriptor
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static int getCneClientFd();

  /*----------------------------------------------------------------------------
   * FUNCTION      setClientFd
   *
   * DESCRIPTION   set the CNEJ file descriptor
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  static void setCneClientFd( int fd );

  // reads a file descriptor until no data is left, no data is saved/returned
  static void clearRead(int fd, void*);

private:

  // the max number of epoll events to be returned at one time
  static const unsigned int EPOLL_SIZE = 4;

  // value matched in CNEJ
  static const int UNSOLICITED_MESSAGE = 1;

  // max that send() can write at once
  static const unsigned int MAX_WRITE_SIZE = 1024;

  CneTimer *timer;

  // file descriptor of the epoll instance
  int epollFd;

  // file descriptor of the eventFd instance (used to wake epoll)
  int eventFd;

  // holds epoll events returned from epoll_wait
  epoll_event events[EPOLL_SIZE];

  // structure for callbacks
  struct ComHandlerObject {
    ComEventCallback callback;
    ComCloseCallback closeCallback;
    void* data;
  };

  // stores mapping of file descriptors to callbacks
  std::map<int, ComHandlerObject> comHandlers;

  // stores list of file descriptors that are ok to work with
  static std::set<int> approvedFd;

  // adds FD to the list of approved file descriptors for I/O
  static void addApprovedFd(int fd);

  // remove FD from the list of approved file descriptors for I/O
  static void remApprovedFd(int fd);

  // helper method to send messages -- will not send messages to FD's
  // not on the approved list
  static void sendMessage(int fd, void const* data, unsigned int size);

  static int cneClientFd; // "CNEJ" socket

};

#endif /* CNE_COM_H */
