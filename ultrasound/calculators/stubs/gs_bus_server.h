/*===========================================================================
          gs_bus_server.h

DESCRIPTION:
Gesture bus server stub API

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#ifndef __GS_BUS_SERVER_H__
#define __GS_BUS_SERVER_H__


/**
 * Gesture bus server interface. This is implemented by GsBusServer.
 */
class GsBusServerInterface
{
public:

  /**
   * Start the server. Returns true if successful.
   */
  virtual bool Start() = 0;

  /**
   * Stop the server.
   */
  virtual void Stop() = 0;

  virtual ~GsBusServerInterface() {}
};

/**
 * Gesture bus server. Implements the socket connection and multicast messaging
 * to the clients. There should be only one instance of this in the system.
 */
class GsBusServer : public GsBusServerInterface
{
public:

  GsBusServer();
  virtual ~GsBusServer();

  /**
   * Start the server. Returns true if successful.
   */
  virtual bool Start();

  /**
   * Start the server using a specified socket address. This is used for test
   * purposes when the actual gesture bus address would disrupt a working system.
   */
  bool Start(char const* busAddress);

  /**
   * Stop the server.
   */
  virtual void Stop();

  /**
   * Get the socket address used to communicate with the server.
   */
  static char const* GetSocketAddress();

private:

  // no copy nor assign because of the implementation pointer
  GsBusServer(GsBusServer const&);
  GsBusServer& operator=(GsBusServer const&);

  // hide the implementation to reduce the include burden on the client
  class GsBusServerImpl* impl;
};


#endif //__GS_BUS_SERVER_H__



