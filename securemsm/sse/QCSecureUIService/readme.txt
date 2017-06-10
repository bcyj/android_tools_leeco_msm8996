/*===========================================================================
 * Copyright(c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

Qualcomm Technologies Secure UI Android Service
===============================================

This service interfaces the native Secure UI daemon and libraries with the
Android framework.
It captures events from the framework and forwards them to the native entities.
The following events are currently captured:
- SCREEN ON
- SCREEN OFF
- HOME SCREEN
- DEVICE UNLOCKED
- PHONE ON HOOK
- PHONE OFF HOOK
Moreover, the phone and screen status are queried synchronously by the native
entities every time they process a request to start Secure Display or Secure
Touch. This service receives such a request and responds with the queried
status.

Moreover, when Secure Display is starting, this service receives such an
information and can perform additional operations within the Android Framework
before the Secure Display session is effectively active.
Currently this information is used to pause, if active, a WiFi Display
connection.
When Secure Display is stopped, similarly a notification is received, and any
service that was stopped can be restored (currently a previously active WiFi
Display connection is resumed).

Secure Display and Touch session will not start unless they can get a
positive response from this service. This implies that they won't be able
to start until this service is started during boot.


Native Communication Layer
==========================

A JNI library linked by this service takes care of the communication with the
other native entities, via a native socket, and exposes the following
interfaces:

private native int init()
    Initialize the native communication layer.

private native void terminate()
    Terminates the native communication layer.

private native byte[] waitForMessage()
    Wait for a message to be received. The received message (4-byte array) is
    returned.

private native byte[] getSource()
    Get the address where the last received message originated. This is needed
    if the message was a command which needs an response to be sent back.

private native int sendResponse(int id, int payload, byte[] remote)
    Send a response to a received command.

public static native int sendNotification(int id, int payload, byte[] remote)
    Send a notification to a remote entity..

The initialization function will automatically connect with the native Secure
Display daemon. Therefore, in order to send a notification to this native
daemon, no remote address needs to be specified (i.e. remote = NULL).
In order to send notifications to the Secure Touch library, on the other end,
its endpoint address needs to be specified in every call.
The Secure Touch library endpoint address, as specified in the code, is:

public static byte[] TOUCH_LIB_ADDR = {0, 's', 'u', 'i', 't', 'c', 'h'};

Supported Messages
------------------

All messages exchanged on the socket are 4 bytes long.

They consist of 4 independent uint8 fields.
- The 1st field carries the message type
- The 2nd field carries the message ID.
- The 3rd field carries an optional (depending on the message)
  additional payload.
- The 4th field is reserved.

The following types of messages are supported:
- Command: requires a response
     - may use the second payload field
- Reponse: to a previously sent command
     - shares the same ID as the command
     - uses the payload field
- Notification: does not require a response
     - may use the payload field

List of messages:
****************************************************************************
- CALL STATUS
   Originator: Android Service (as Notification), SecureUILib (as Command)
   Type: Notification or command
   Payload (for the notification and for the response):
     OK if no call is in progress, ABORT if a call is in progress
****************************************************************************
- ABORT
   Originator: Android Service
   Type: Notification
   Payload: None
****************************************************************************
- SCREEN STATUS
   Originator: Android Service (as Notification), SecureUILib (as Command)
   Type: Notification or command
   Payload (for the notification and for the response):
     OK if screen is on, ABORT if screen is off
****************************************************************************
- SD STARTING
   Originator: Secure Display daemon
   Type: Command / Response
   Payload:
     Command: None
     Response: Screen rotation if OK, ABORT otherwise
      Screen rotation is encoded as follows:
          0 --> 0x00
         90 --> 0x10
        180 --> 0x20
        270 --> 0x30
      I.e. the OK message is delivered in the LS nibble, the rotation
      in the MS nibble.
****************************************************************************
 - SD STOPPED
   Originator: Secure Display daemon
   Type: Notification
   Payload: None
****************************************************************************

The codes for the commands and payloads are defined in the service as follows:

Message type:
private static final int SUI_MSG_CMD_MSK        = 0x08;
private static final int SUI_MSG_RSP_MSK        = 0x04;
private static final int SUI_MSG_NOT_MSK        = 0x02;

Message ID:
public static final int SUI_MSG_ID_CALL_STATUS = 0x01;
public static final int SUI_MSG_ID_ABORT       = 0x02;
public static final int SUI_MSG_ID_SCREEN_STATUS = 0x03;
public static final int SUI_MSG_ID_SD_STARTING = 0x10;
public static final int SUI_MSG_ID_SD_STOPPED  = 0x11;

Message payload:
public static final int SUI_MSG_RET_OK         = 0x00;
public static final int SUI_MSG_RET_ABORT      = 0x01;
public static final int SUI_MSG_RET_NOP        = 0x0F;
