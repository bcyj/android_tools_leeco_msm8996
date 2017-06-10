/*===========================================================================
                           gesture_bus_stub.cpp

DESCRIPTION: Implements a stub for the gesture bus.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "gesture_bus_stub"
#include "gs_bus_client.h"
#include "usf_log.h"
#include "gs_bus_server.h"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*============================================================================
  CONSTRUCTOR:  GsBusClient
============================================================================*/
/**
  See function definition in header file.
 */
GsBusClient::GsBusClient()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  DESTRUCTOR:  GsBusClient
============================================================================*/
/**
  See function definition in header file.
 */
GsBusClient::~GsBusClient()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  Connect
============================================================================*/
/**
  See function definition in header file.
 */
bool GsBusClient::Connect(char const* sourceName,
                          GsBusAbstractListener& listener)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return true;
}

/*============================================================================
  FUNCTION:  Connect
============================================================================*/
/**
  See function definition in header file.
 */
bool GsBusClient::Connect(char const* sourceName,
                          GsBusAbstractListener& listener,
                          char const *busAddress)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return true;
}

/*============================================================================
  FUNCTION:  Disconnect
============================================================================*/
/**
  See function definition in header file.
 */
void GsBusClient::Disconnect()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  SendMessage
============================================================================*/
/**
  See function definition in header file.
 */
bool GsBusClient::SendMessage(int dispatchIndex,
                              void const *message,
                              size_t size)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return true;
}


/*============================================================================
  CONSTRUCTOR:  GsBusServer
============================================================================*/
/**
  See function definition in header file.
 */
GsBusServer::GsBusServer()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  DESTRUCTOR:  GsBusServer
============================================================================*/
/**
  See function definition in header file.
 */
GsBusServer::~GsBusServer()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}
/*============================================================================
  FUNCTION:  Start
============================================================================*/
/**
  See function definition in header file.
 */
bool GsBusServer::Start()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return true;
}

/*============================================================================
  FUNCTION:  Start
============================================================================*/
/**
  See function definition in header file.
 */
bool GsBusServer::Start(char const *busAddress)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return true;
}

/*============================================================================
  FUNCTION:  Stop
============================================================================*/
/**
  See function definition in header file.
 */
void GsBusServer::Stop()
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  CONSTRUCTOR:  GsBusServer
============================================================================*/
/**
  See function definition in header file.
 */
GsBusServer::GsBusServer(GsBusServer const&)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}
/*============================================================================
  FUNCTION:  operator=
============================================================================*/
/**
  See function definition in header file.
 */
GsBusServer& GsBusServer::operator=(GsBusServer const& bus_server)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return *this;
}
