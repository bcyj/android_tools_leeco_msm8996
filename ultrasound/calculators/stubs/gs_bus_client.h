/*===========================================================================
                           gs_bus_client.h

DESCRIPTION: Contains function definitions for gesture bus.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __GS_BUS_CLIENT_H__
#define __GS_BUS_CLIENT_H__

#include "gesture_defs.h"




/**
 * Concrete gesture bus client.
 *
 * The gesture bus provides communications among gestures sources running in
 * different processes. Messages posted to the gesture bus are received by all
 * clients connected to the bus. The Gesture Manager (a client of the bus)
 * listens for Outcomes posted to the bus and forwards them up the SW stack to
 * the application. Configuration information is provided by a configuration set
 * which selects, among other things, the mode of operation of the gesture
 * system.
 *
 * Specialized communications between different gesture sources (eg, between
 * camera and ultrasound) is provided by defining messages for that
 * communication in GsBusListener.
 *
 * Inherit from GsBusListener and override the callbacks to receive gesture bus
 * messages, eg:
 *
 *    virtual void OnMessage(GsConfigurationParams const& message)
 *    {
 *      // handle configuration messages
 *    }
 *
 * Message callbacks occur asynchronously in a separate thread context.
 *
 *
 * Example usage:
 *
 *  class MyClient : public GsBusListener
 *  {
 *    void Connect()
 *    {
 *      client.Connect("MySourceName", *this);
 *    }
 *
 *    void PostOutcome(GsBusOutcome const& outcome)
 *    {
 *      client.PostMessage(outcome);
 *    }
 *
 *    void Disconnect()
 *    {
 *      client.Disconnect();
 *    }
 *
 *    virtual void OnMessage(GsConfigurationParams const& parm)
 *    {
 *      // callback to set configuration parameters
 *    }
 *
 *    GsBusClient client;
 *  };
 */
class GsBusClient : public GsBusAbstractClient
{
public:

  GsBusClient();
  virtual ~GsBusClient();

  // implementation of GsBusAbstractClient
  virtual bool Connect(char const* sourceName, GsBusAbstractListener& listener);
  virtual void Disconnect();

  // connect using a specific bus address for test purposes
  bool Connect(char const* sourceName, GsBusAbstractListener& listener, char const* busAddress);

private:

  // no copy nor assign because of the implementation pointer
  GsBusClient(GsBusClient const&);
  GsBusClient& operator=(GsBusClient const&);

  virtual bool SendMessage(
   int dispatchIndex,
   void const* message,
   size_t size);

  // hide the implementation to reduce the include burden
  class GsBusClientImpl* impl;
};


#endif //__GS_BUS_CLIENT_H__

