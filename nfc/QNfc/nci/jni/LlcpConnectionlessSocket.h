/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#pragma once

extern "C"
{
#include "nfa_api.h"
#include "nfa_p2p_api.h"
}

#include "Mutex.h"
#include "SyncEvent.h"

#include <string>
#include <vector>

/**
 * A socket for sending and receiving PDUs over connectionless LLCP link.
 * Each of the sockets are registered to have an unique local SAP either automatically by the NFC stack
 * or explicitly by the application that instantiates a socket.
 * This implementation is completely thread-safe to use.
 */
class LlcpConnectionlessSocket {
public:

  /**
   * Opens a new client socket that will automatically get the next free local SAP assigned to it.
   *
   * @return The client socket instance or 0 if opening the socket failed.
   */
  static LlcpConnectionlessSocket* open();

  /**
   * Opens a new connectionless LLCP service socket that is registered to the speficied local SAP and will provide the specified
   * local service name for remote peers. However, if the local service name parameter is null or empty string, the socket is
   * registered as a client socket to have the next free local SAP and the parameter SAP is ignored.
   *
   * @param localServiceName The name of the service accessible via the socket. Can be null or empty string.
   * @paral localSap The local SAP of the socket in the range 11h-1Fh. If the service name is not provided, the local SAP is ignored and
   *  the next free SAP will be assigned for the socket.
   *
   * @return The LlcpConnectionless socket or 0 if the socket could not be opened.
   */
  static LlcpConnectionlessSocket* open(const char* localServiceName, int localSap);

  /**
   * Gets the LlcpConnectionlessSocket instance corresponding to the specified local SAP.
   *
   * @param localSap The local SAP number or handle of the socket.
   *
   * @return The pointer at the LlcpConnectionlessSocket instance or 0 if the socket is not open.
   */
  static LlcpConnectionlessSocket* getSocket(int localSap);

  /**
   * Checks whether the socket is open at the moment.
   *
   * @return true if the socket is open, false if it is closed.
   */
  bool isOpen();

  /**
   * Gets the local SAP assigned to the socket.
   *
   * @return The local SAP assigned to the socket or -1 if the socket is not open.
   */
  int getLocalSap();

  /**
   * Sends a PDU data packet to the specified remote SAP.
   *
   * @param remoteSap The remote SAP where the PDU is sent to.
   * @param data The vector containing the PDU.
   *
   * @return true if the PDU was sent successfully or false if an error occured during sending.
   */
  bool send(int remoteSap, const std::vector<unsigned char>& data);

  /**
   * Sends a PDU data packet to a remote peer the provides the specified service name.
   *
   * @param remoteServiceName The name of the service provided by the remote peer.
   * @param data The vector containing the PDU.
   *
   * @return true if the PDU was sent successfully or false if an error occured during sending.
   */
  bool send(const char* remoteServiceName, const std::vector<unsigned char>& data);

  /**
   * Receives a PDU from a remote peer. This method will block indefinitely until it receives the PDU or
   * the socket gets closed.
   *
   * @param data The data vector where the received PDU is stored.
   *             The vector is automatically resized to match the size of the PDU.
   *
   * @return The remote SAP of the remote peer or -1 if the socket was closed before receiving the PDU.
   */
  int receive(std::vector<unsigned char>& data);

  /**
   * Closes the socket.
   */
  void close();

  /**
   * Closes all open sockets. Called when the NFC stack is shut down.
   */
  static void closeAllSockets();

private:
  /**
   * An array containing all connectionless LLCP sockets.
   */
  static LlcpConnectionlessSocket socketMap[];

  /**
   * The main mutex for making sure that all socket instances are thread-safe.
   */
  static Mutex mainMutex;

  /**
   * A mutex for synchronizing the socket registration sequence.
   */
  static SyncEvent regEvent;

  /**
   * The NFA handle used to deliver the handle of the registered socket.
   */
  static tNFA_HANDLE regHandle;

  /**
   * A mutex for synchronizing the SDP request sequence.
   */
  static SyncEvent sdpEvent;

  /**
   * Gets the socket instance that corresponds to the specified NFA handle.
   *
   * @param handle The NFA handle.
   *
   * @return The socket instance or 0 if the handle is invalid.
   */
 // static LlcpConnectionlessSocket* getSocket(tNFA_HANDLE handle);

  /**
   * Notifies about NFA P2P events reported by the NFC stack.
   */
  static void notifyNfaP2pEvent(tNFA_P2P_EVT event, tNFA_P2P_EVT_DATA *eventData);

  /**
   * The socket's unique NFA handle given by the NFC stack on registration.
   */
  tNFA_HANDLE socketHandle;

  /**
   * The local MIU.
   */
  int localMiu;

  /**
   * The remote MIU.
   */
  int remoteMiu;

  /**
   * Remote SAP that is retrieved by SDP, 0 if the SDP request fails.
   */
  UINT8 remoteSapForWrite;

  /**
   * Current remote service name that was found with SDP.
   */
  std::string currRemoteServiceName;

  /**
   * A mutex for synchronizing PDU receive operation.
   */
  SyncEvent* readEvent;

  /**
   * Remote SAP of the peer that is sending PDU to the socket.
   */
  UINT8 remoteSapForRead;

  /**
   * Initialized the socket.
   */
  LlcpConnectionlessSocket();

  /**
   * Prevented use of copy constructor.
   */
  LlcpConnectionlessSocket(const LlcpConnectionlessSocket&);

  /**
   * Prevented use of assignment operator.
   */
  LlcpConnectionlessSocket& operator=(const LlcpConnectionlessSocket&);

  /**
   * Sends the specified data packet to the remove SAP port.
   *
   * @param remoteSAP The remote SAP.
   * @param data The data to send.
   *
   * @return true if the sending was successful, false otherwise.
   */
  bool sendPDU(int remoteSap, const std::vector<unsigned char>& data);

};
