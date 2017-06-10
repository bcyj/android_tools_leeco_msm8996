#ifndef __UIBC_PACKET_TRANSMITTER_H__
#define __UIBC_PACKET_TRANSMITTER_H__
/*==============================================================================
  *       UIBCPacketTransmitter.cpp
  *
  *  DESCRIPTION:
  *       Transmit the UIBC packets to source device
  *
  *
  *  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  *  Qualcomm Technologies Proprietary and Confidential.
  *==============================================================================*/

/* =======================================================================
                             Edit History
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "AEEstd.h"
#include "MMMemory.h"
#include "UIBCPacketizer.h"
#include "qmmList.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */


/* =======================================================================
**                        Class Declarations
** ======================================================================= */
class UIBCPacketTransmitter
{
public:

  UIBCPacketTransmitter(uint16 sourcePort, uint32 sourceIPaddr, uint32 negotiated_height, uint32  negotiated_width);

  virtual ~UIBCPacketTransmitter();

  UIBC_sink_status_t sendEvent(WFD_uibc_event_t* event);

  UIBC_sink_status_t start();
  UIBC_sink_status_t stop();

private:

  typedef enum
  {
    UIBC_NO_INIT = 0,     //Component is not yet initialized
    UIBC_INIT,            //Component is initialized
    UIBC_CONNECTED,       //Established the connection
    UIBC_STOP,            //Component is stopped
  }uibc_state_t;

  typedef struct
  {
    QMM_ListLinkType link;
    WFD_uibc_event_t event;
  } uibc_events_list_link_t;


  /*
     *Following values represents the user data corresponding to the signals defined
     */
  static const uint32 THREAD_CONNECT_SIGNAL;      //user data for m_pConnectSignal
  static const uint32 THREAD_EXIT_SIGNAL;         //user data for m_pExitSignal
  static const uint32 THREAD_SEND_EVENT_SIGNAL;   //user data for m_pSendEventSignal
  static const uint32 THREAD_STOP_SEND_SIGNAL;    //user data for m_pStopSendSignal

  /*port number on which source accpets connection*/
  uint16 m_nSourcePort;

  /*IPV4 address of source*/
  uint32 m_nSourceIPaddr;
  uint32 m_negotiated_height;
  uint32 m_negotiated_width;

  uibc_state_t m_eUIBCState;

  int32 m_nSocket;

  MM_HANDLE m_pUIBCThread;

  MM_HANDLE m_pSignalQ;
  MM_HANDLE m_pConnectSignal;
  MM_HANDLE m_pExitSignal;
  MM_HANDLE m_pSendEventSignal;
  MM_HANDLE m_pStopSendSingal;

  MM_HANDLE m_pConnectTimer;

  QMM_ListHandleType m_hEventsList;

  UIBCPacketizer* m_pUIBCPacketizer;

  /*buffer to hold uibc raw packet*/
  uint8* m_pUIBCBuffer;


  static int uibcThreadEntry( void* ptr );
  static void connectTimerHandler( void* ptr);

  void uibcThreadWorker();
  int32 makeConnection();

  void processEventsList();
  void flushEventsList();

  int32 sendPacket(uint8* pUIBCPacket, int32 packetLength );

  void init();
  int32 createSocket();

  void release();
};
#endif/*__UIBC_PACKET_TRANSMITTER_H__ */
