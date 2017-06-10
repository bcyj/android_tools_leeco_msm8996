/*==============================================================================
 *  @file UIBCInputReceiver.h
 *
 *  @par  DESCRIPTION:
 *        Class Definition of the UIBC Input Receiver(Wifi Display Source)
 *        Contains interfaces and members to receive UIBC inputs over TCP link
 *        and it interfaces with modules that parser/inject those inputs
 *
 *  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //source/qcom/qct/multimedia2/Video/wfd/uibc/main/latest/source/inc/UIBCInputReceiver.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$


===============================================================================*/
#ifndef UIBC_INPUT_RECEIVER_H
#define UIBC_INPUT_RECEIVER_H

#include "MMMemory.h"
#include "AEEstd.h"
#include "UIBCDefs.h"

#define UIBC_THREAD_STACK_SIZE                  8192
#define UIBC_SOCKET_RECEIVE_TIME_OUT            100000
#define UIBC_PACKET_HEADER_LEN                  4
#define UIBC_TS_LEN                             2
#define UIBC_GENERIC_IE_HEADER_LEN              3
#define UIBC_GENERIC_IE_TOUCH_MAX_LEN           (1 + 5 * UIBC_MAX_TOUCH_INPUTS)
#define UIBC_GENERIC_IE_KEY_MAX_LEN             5
/*
  Not really sure what the payload can actually be since for generic it can be
  pegged at a maximum of ~35 bytes but for HID the report descriptor might be
  fairly large (default keyboard report descriptor is ~65 bytes). For complex
  HID devices, it might run into more. But since report descriptors are stored
  on device firmware, they are conservatively sized. For now pegging this value
  to a maximum of 100, might need to be tweaked upwards
*/
#define UIBC_PACKET_MAXIMUM_SIZE                100

//Forward Declarations
class  UIBCInputInjector;
class  UIBCInputParser;
class  UIBCHIDInjector;

class UIBCInputReceiver
{
  public:

  //Input Receiver constructor
  UIBCInputReceiver();
  virtual ~UIBCInputReceiver();

  //Start listening for Inputs, UIBC Enabled
  boolean Start();
  //Stop listening to Inputs, UIBC Disabled or some error
  boolean Stop();
  //Sets UIBC Negotiated Capability, would help in Sanity Checks
  void SetUIBCCapability(WFD_uibc_capability_t*);
  //Gets UIBC Negotiated Capability
  void GetUIBCCapability(WFD_uibc_capability_config_t*);

  void RegisterCallback (wfd_uibc_attach_cb     Attach,
                         wfd_uibc_send_event_cb SendEvent,
                         wfd_uibc_hid_event_cb sendHID,
                         void*                  ClientData);

  boolean                                  m_bListenForConn;

  private:

  enum UIBCThreadState
  {
    UIBC_NO_INIT = 0,     //UIBC Thread is not yet initialized
    UIBC_INITIALIZED,     //UIBC Thread is initialized
    UIBC_FETCHING_DATA,   //UIBC Thread starts fetching data
    UIBC_STOPPED,         //UIBC Thread stopped fetching data
  };

  enum UIBCInputInjectMode
  {
    NONE,
    USER,
    KERNEL
  };

  UIBCThreadState        m_eUIBCState;
  UIBCInputInjectMode    m_eUIBCInputInjectMode;

  MM_HANDLE              m_pUIBCThread;
  MM_HANDLE              m_pSignalQ;
  MM_HANDLE              m_pFetchDataSignal;
  MM_HANDLE              m_pStopSignal;
  MM_HANDLE              m_pExitSignal;
  MM_HANDLE              m_pAcceptSignal;

  wfd_uibc_attach_cb     m_pAttachVMCB;
  wfd_uibc_send_event_cb m_pSendEventCB;
  wfd_uibc_hid_event_cb  m_pSendHIDEventCB;
  void*                  m_pClientData;

  // Following values are the user data corresponding to m_pFetchDataSignal,
  // m_pExitSignal respectively

  static const uint32 FETCH_DATA_SIGNAL;
  static const uint32 ACCEPT_CONNECTION_SIGNAL;
  static const uint32 THREAD_EXIT_SIGNAL;
  static const uint32 FETCH_DATA_STOP_SIGNAL;
  int32                 m_nSocket;
  //Following is the accepted data socket
  int32                 m_nTCPDataSocket;
  //Buffer to store the received TCP Packet
  uint8*                 m_UIBCPacket;

  //Following 2 are packet indices. negative value indicates they are not valid
  int32                m_packetReadIndex;
  int32                m_packetWriteIndex;
  //Stores the negotiated capability
  WFD_uibc_capability_t m_pNegotiatedCapability;
  //Handle to the Input Injector Class
  UIBCInputInjector     *m_pInjector;
  //Handle to UIBC Input Parser
  UIBCInputParser       *m_pParser;
  UIBCHIDInjector       *m_pHIDInjector;

  static int   UibcThreadEntry( void* ptr );
  void         UibcThreadWorker();
  void         Init();
  int32        CreateSocket(int16 port);
  int32        SetSocketOptions(int32 socket);
  int32        AcceptTCPConnection(int32 tcpSocket);
  int32        FetchData();
  int32        ReceiveUIBCPacket(uint8* , uint32 nFetchSize = UIBC_PACKET_MAXIMUM_SIZE);
  boolean      ParseAndInjectInput();
  void         CloseDataSource();
};

#endif /*UIBC_INPUT_RECEIVER_H*/

