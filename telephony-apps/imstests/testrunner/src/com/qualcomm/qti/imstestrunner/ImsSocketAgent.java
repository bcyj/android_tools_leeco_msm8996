/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.qti.imstestrunner;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Random;

import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;

import android.os.HandlerThread;
import android.os.Message;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;

import android.util.Log;
import java.io.DataOutputStream;

public class ImsSocketAgent {
    public static final String LOGTAG = "ImsSocketAgent";
    /*
     * The TEST_* constants must match constants defined in telephony
     * -apps/ims/tests/ims/src/com/qualcomm/qti/ims/tests/ImsTestHandler.java
     */

    // (Test options)
    public static final int TEST_BASIC_MO_MT_SUCC = 0;

    public static final boolean testMode = true;

    static final int MAX_COMMAND_BYTES = (8 * 1024);
    static final String SOCKET_NAME_IF = "imstestrunnersocket";
    static boolean IF_LOGD = true;;

    HandlerThread mSenderThread;
    ImsSendResponse mSender;
    LocalSocket mSocket = null;

    private static final int FAIL = 0;
    private static final int SUCCESS = 1;
    private static ImsSocketAgent mInstance = null;
    private static final int PDU_LENGTH_OFFSET = 4;
    private static final int MSG_TAG_LENGTH = 1;
    private LocalServerSocket serverSocket = null;
    private byte[] mBuffer;
    private Thread mServerThread;
    private boolean mmtCallMade = false;
    private int mTestOption;
    private ResponseGenerator mResponseGenerator;
    private boolean unsolImsNetworkStateChanged = false;
    private boolean unsolRadioStateChanged = false;
    private boolean mIncomingCallPolled = false;

    /**
     * @return the current instance of ImsSocketAgent if it exists; if it is
     *         currently null, it creates a new one.
     */
    public static ImsSocketAgent getInstance() {
        ImsSocketAgent ret;
        if (mInstance == null) {
            mInstance = newInstance();
        }
        return mInstance;
    }

    /**
     * @return a new instance of ImsSocketAgent; should only be called by
     *         ImsSocketAgent.getInstance()
     */
    private static ImsSocketAgent newInstance() {
        ImsSocketAgent ret = new ImsSocketAgent();
        return ret;
    }

    /**
     * Resets the socket connection between ImsSocketAgent and ImsSenderRxr
     */
    public static void reset() {
        try {
            mInstance.serverSocket.close();
        } catch (IOException e) {
            mInstance.logDebug("IOException in reset()");
        }
        mInstance = newInstance();
    }

    private ImsSocketAgent() {
        logDebug("ImsSocketAgent Constructor");

        // if we're not using instrumentation test runner, set test option from
        // here
        boolean usingInstr = SystemProperties.getBoolean("persist.radio.testing", false);
        logDebug("Using instrumentation: " + usingInstr);
        if (!usingInstr) {
            setTestOption(TEST_BASIC_MO_MT_SUCC);
        }

        mBuffer = new byte[MAX_COMMAND_BYTES];
        try {
            serverSocket = new LocalServerSocket(SOCKET_NAME_IF);
        } catch (IOException e) {
            e.printStackTrace();
            logDebug("Exception in creating Server Socket");
        }

        sendThread();
        connectSocketServer();
        logDebug("ImsSocketAgent Constructor sender" + mSender);
    }

    private void logDebug(String input) {
        if (IF_LOGD)
            Log.d(LOGTAG, input);
    }

    public ResponseGenerator getResponseGenerator() {
        return mResponseGenerator;
    }

    /**
     * Sets test option to number passed in.
     *
     * @param testOption is an int that represents the test option. This int
     *            should be a final int defined in ImsSocketAgent beginning with
     *            TEST_*
     */
    public void setTestOption(int testOption) {
        mTestOption = testOption;
        mResponseGenerator = ResponseGeneratorFactory.getInstance(testOption);
    }

    class Server implements Runnable {
        public void run() {
            int retryCount = 0;
            boolean killMe = false;
            while (!mServerThread.isInterrupted()) {
                LocalSocket socket = null;
                LocalSocketAddress l;
                try {
                    logDebug("ImsSocketAgent connectSocketServer before accept -->" + socket);
                    socket = serverSocket.accept();
                } catch (IOException ex) {
                    ex.printStackTrace();
                    logDebug("ImsSocketAgent connectSocketServer" + ex);
                    if (retryCount == 80) {
                        logDebug("Couldn't find socket after " + retryCount
                                + " times, continuing to retry silently");
                        try {
                            socket.close();
                            killMe = true;
                            return;
                        } catch (IOException e) {
                            Log.e(LOGTAG, "IOException", ex);
                            return;
                        }
                    } else if (retryCount > 0 && retryCount < 80) {
                        Log.i(LOGTAG,
                                "Couldn't find socket; retrying after timeout" + retryCount);
                    }
                    retryCount++;
                    continue;
                }
                retryCount = 0;
                mSocket = socket;
                if (mSocket != null) {
                    // if (mSender == null) {
                    // Looper looper = mSenderThread.getLooper();
                    // mSender = new ImsSendResponse(looper, mSocket);
                    // }
                    startSocketServer(mSocket);
                }
            }
        }
    }

    private void connectSocketServer() {
        Server server = new Server();
        mServerThread = new Thread(server);
        mServerThread.start();
    }

    private void startSocketServer(LocalSocket socket) {
        logDebug("startSocketServer  started 1");
        int length = 0;
        try {
            InputStream in = socket.getInputStream();
            while (true) {
                logDebug("startSocketServer started input stream available bytes->"
                        + in.available());
                length = readMessage(in, mBuffer);
                logDebug("startSocketServer started length" + length);
                if (length < 0) {
                    break;
                }
                Log.v(LOGTAG, "Read packet: " + length + " bytes");
                if (length > 0) {
                    logDebug("startSocketServer started 5");
                    processRequestMessage(mBuffer, length);
                }
                logDebug("Read packet: " + length + " bytes");
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        } catch (Throwable tr) {
            Log.e(LOGTAG, "Uncaught exception read length=" + length +
                    "Exception:" + tr.toString());
        }
        Log.i(LOGTAG, "Disconnected from socket");

        try {
            socket.close();
        } catch (IOException ex) {
        }

        socket = null;
    }

    private int readMessage(InputStream is, byte[] buffer)
            throws IOException {
        int countRead;
        int offset;
        int remaining;
        int messageLength;
        // First, read in the length of the message
        offset = 0;
        remaining = 4;
        do {
            logDebug("readMessage");
            countRead = is.read(buffer, offset, remaining);
            logDebug("readMessage countRead: " + countRead);
            if (countRead < 0) {
                logDebug("Hit EOS reading message length");
                return -1;
            }
            offset += countRead;
            remaining -= countRead;
            logDebug("readMessage remaining: " + remaining);
        } while (remaining > 0);

        messageLength = ((buffer[0] & 0xff) << 24)
                | ((buffer[1] & 0xff) << 16)
                | ((buffer[2] & 0xff) << 8)
                | (buffer[3] & 0xff);
        logDebug("readMessage messageLength" + messageLength);
        // Then, re-use the buffer and read in the message itself
        offset = 0;
        remaining = messageLength;
        do {
            countRead = is.read(buffer, offset, remaining);
            if (countRead < 0) {
                Log.e(LOGTAG, "Hit EOS reading message.  messageLength == " + messageLength
                        + " remaining=" + remaining);
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        return messageLength;
    }

    private void logData(byte[] data, int length) {
        StringBuilder s = new StringBuilder();
        s.append("[");
        for (int i = 0; i < length; i++) {
            s.append(i + ":" + data[i]);
            if (i < length - 1) {
                s.append(", ");
            }
        }
        s.append("]");
        logDebug(s.toString());
    }

    private void processRequestMessage(byte[] data, int length) {
        int token;
        int type;
        int msgId;
        Log.v(LOGTAG, "processResponse");
        logData(data, length);
        ImsQmiIF.MsgTag tag = ImsUnpackMessage(data, length);
        if (tag != null) {
            if (tag.hasToken()) {
                token = tag.getToken();
                Log.v(LOGTAG, "**************Message Tag token: ********** " + token);
            }
            if (tag.hasType()) {
                type = tag.getType();
                Log.v(LOGTAG, "************Message Tag type:************* " + type);
            }
            if (tag.hasId()) {
                msgId = tag.getId();
                Log.v(LOGTAG, "************Message ID:***********" + msgId);
            }
        }
    }

    private ImsQmiIF.MsgTag ImsUnpackMessage(byte[] data, int length) {
        // Parse tag length from first one byte
        int msglen = 1;
        int startIndex = 0;
        int endIndex = startIndex + msglen;
        byte[] msg = null;
        ImsQmiIF.MsgTag tag = null;
        int token = 0;
        int type = 0;
        int msgId = 0;
        int error = 0;
        log(" ImsUnpackMessage processResponse");
        if (testMode) {
            logData(data, length);
        }

        if (endIndex <= length) {
            msglen = data[startIndex];

            // Parse tag

            startIndex = endIndex;
            endIndex = startIndex + msglen;

            if ((endIndex <= length) && (msglen > 0)) {

                msg = new byte[msglen];
                log("msglen = " + msglen);
                try {
                    System.arraycopy(data, startIndex, msg, 0, msglen);
                    // Convert tag in bytes to local data structure
                    tag = ImsQmiIF.MsgTag.parseFrom(msg);
                } catch (IndexOutOfBoundsException ex) {
                    log(" IndexOutOfBoundsException while parsing msg tag");
                    return tag;
                } catch (ArrayStoreException ex) {
                    log(" ArrayStoreException while parsing msg tag");
                    return tag;
                } catch (NullPointerException ex) {
                    log(" NullPointerException while parsing msg tag");
                    return tag;
                } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                    log("InvalidProtocolBufferException while parsing msg tag");
                    return tag;
                }
                log(" Tag " + tag.getToken() + " " + tag.getType() + " "
                        + tag.getId() + " " + tag.getError());
                // Parse message content if present
                startIndex = endIndex;
                msglen = length - startIndex;
                endIndex = startIndex + msglen;
                log("{startIndex == " + startIndex + ", msglen == " + msglen + ", endIndex == "
                        + endIndex
                        + ", length == " + length + "}");
                msg = null;
                if ((endIndex <= length) && (msglen > 0)) {
                    msg = new byte[msglen];
                    try {
                        log("array copy to parse message content furhter ");
                        System.arraycopy(data, startIndex, msg, 0, msglen);
                    } catch (IndexOutOfBoundsException ex) {
                        log(" IndexOutOfBoundsException while parsing msg tag");
                    } catch (ArrayStoreException ex) {
                        log(" ArrayStoreException while parsing msg tag");
                    } catch (NullPointerException ex) {
                        log(" NullPointerException while parsing msg tag");
                    }

                }
                if (tag != null) {
                    if (tag.hasToken()) {
                        token = tag.getToken();
                        Log.v(LOGTAG, "Message Tag token: " + token);
                    }
                    if (tag.hasType()) {
                        type = tag.getType();
                        Log.v(LOGTAG, "Message Tag type: " + type);
                    }
                    if (tag.hasId()) {
                        msgId = tag.getId();
                    }
                    if (tag.hasError()) {
                        error = tag.getError();
                    }
                }
                sendImsResponse(msgId, token, type, error, msg, msglen);
            }
        }
        return tag;
    }

    private void sendImsResponse(int msgId, int msgToken, int msgType, int error, byte[] msg,
            int length) {
        log("Send response");
        if (msg != null && msg.length > MAX_COMMAND_BYTES) {
            throw new RuntimeException("Message is larger than max bytes allowed! " +
                    msg.length);
        } else {
            logDebug("packResponseMessage :: msgId =" + msgId + "length =" + length);
            packResponseMessage(msgId, msgToken, msgType, error, msg, length);
        }
    }

    private void packResponseMessage(int msgId, int msgToken, int msgType, int error, byte[] msg,
            int length) {
        if (!unsolImsNetworkStateChanged) {
            unsolImsNetworkStateChanged = true;
            unsolImsNetworkStateChanged(msgToken);
        }
        switch (msgId) {
            case ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE:
                respondImsRegistrationState(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION:
                respondSetSuppSvcNotification(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS:
                respondQueryServiceStatus(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_GET_CURRENT_CALLS:
                respondGetCurrentCalls(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_DIAL:
                respondDial(msgId, msgToken, msg);
                break;
            case ImsQmiIF.REQUEST_ANSWER:
                respondAnswer(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_HANGUP:
                respondHangup(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
                respondHangupFgResumeBg(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_LAST_CALL_FAIL_CAUSE:
                respondLastCallFailCause(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                respondSwitchWaitingOrHoldingAndActive(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_ADD_PARTICIPANT:
                respondAddParticipant(msgId, msgToken, msg);
                break;
            case ImsQmiIF.REQUEST_HANGUP_WAITING_OR_BACKGROUND:
                respondHangupWaitingOrBackground(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_HOLD:
                respondHold(msgId, msgToken);
                break;
            case ImsQmiIF.REQUEST_RESUME:
                respondResume(msgId, msgToken);
                break;
            default:
                sendResponse(msgId, msgToken, null);
                log("Unrecognized message ID sending dummy response: " + msgId);
                break;
        }
    }

    public IFResponse encodeMsg(int id, int tokenid, int msgType, int error, byte[] msg) {

        int msgLen = 0;
        int index = 0;
        int totalPacketLen = 0;
        IFResponse rr = IFResponse.obtain(id, tokenid);
        ImsQmiIF.MsgTag msgtag;
        msgtag = new ImsQmiIF.MsgTag();
        msgtag.setToken(rr.mSerial);
        msgtag.setType(msgType);
        msgtag.setId(rr.mRequest);
        msgtag.setError(error);
        byte[] tagb = msgtag.toByteArray();

        if (msg != null) {
            msgLen = msg.length;
        }

        // data is the byte stream that will be sent across the socket
        // byte 0..3 = total msg length (i.e. tag length + msglen)
        // byte 4 = msgtag length
        // byte 5..tagb.length = msgtag in bytes
        // byte 5+tagb.length..msglen = msg in bytes
        totalPacketLen = PDU_LENGTH_OFFSET + tagb.length + msgLen + MSG_TAG_LENGTH;
        rr.mData = new byte[totalPacketLen];

        // length in big endian
        rr.mData[index++] = rr.mData[index++] = 0;
        rr.mData[index++] = (byte) (((totalPacketLen - PDU_LENGTH_OFFSET) >> 8) & 0xff);
        rr.mData[index++] = (byte) (((totalPacketLen - PDU_LENGTH_OFFSET)) & 0xff);

        rr.mData[index++] = (byte) tagb.length;

        try {
            System.arraycopy(tagb, 0, rr.mData, index, tagb.length);
        } catch (IndexOutOfBoundsException ex) {
            log(" IndexOutOfBoundsException while encoding msg tag");
        } catch (ArrayStoreException ex) {
            log(" ArrayStoreException while parsing msg tag");
        } catch (NullPointerException ex) {
            log(" NullPointerException while parsing msg tag");
        }

        if (msgLen > 0) {
            try {
                System.arraycopy(msg, 0, rr.mData, (index + tagb.length), msgLen);
            } catch (IndexOutOfBoundsException ex) {
                log(" IndexOutOfBoundsException while encoding msg");
            } catch (ArrayStoreException ex) {
                log(" ArrayStoreException while parsing msg");
            } catch (NullPointerException ex) {
                log(" NullPointerException while parsing msg");
            }
        }

        if (IF_LOGD) {
            log(rr.serialString() + "> " + msgIdToString(rr.mRequest));
            logData(rr.mData, rr.mData.length);
        }

        return rr;
    }

    public int makeMtCall(int msgToken, int state) {
        mResponseGenerator.callStateIncomingOrWaiting(state);
        byte[] response = null;
        sendUnsolResponse(ImsQmiIF.UNSOL_CALL_RING, msgToken, response);
        response = mResponseGenerator.generateUnsolMtCallResponse();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        return mResponseGenerator.getMtCallType();
    }

    private void respondQueryServiceStatus(int msgId, int msgToken) {
        log("REQUEST_QUERY_SERVICE_STATUS received");
        byte[] response = mResponseGenerator.generateSrvStatusList();
        sendResponse(msgId, msgToken, response);
        log("REQUEST_QUERY_SERVICE_STATUS responded to");
    }

    private void respondImsRegistrationState(int msgId, int msgToken) {
        log("REQUEST_IMS_REGISTRATION STATE received");
        byte[] response = mResponseGenerator.generateImsRegistrationState();
        sendResponse(msgId, msgToken, response);
        log("REQUEST_IMS_REGISTRATION STATE responded to");
    }

    private void respondSetSuppSvcNotification(int msgId, int msgToken) {
        log("REQUEST_SET_SUPP_SVC_NOTIFICATION received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        log("REQUEST_SET_SUPP_SVC_NOTIFICATION responded to");
    }

    private void respondLastCallFailCause(int msgId, int msgToken) {
        log("REQUEST_LAST_CALL_FAIL_CAUSE received.");
        byte[] response = mResponseGenerator.generateLastCallFailCause();
        sendResponse(msgId, msgToken, response);
        mResponseGenerator.clearCallList();
        log("REQUEST_LAST_CALL_FAIL_CAUSE responded to");
    }

    private void respondDial(int msgId, int msgToken, byte[] msg) {
        log("REQUEST_DIAL received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        boolean dial = mResponseGenerator.callStateDialing(msg);
        log("REQUEST_DIAL responded to");
        // in Android L, unsols are sent here.
        if (mResponseGenerator.getIsAndroidL()) {
            unsolStateChangeToDialing(msgToken);
            unsolStateChangeDialingToAlerting(msgToken);
            unsolStateChangeAlertingToActive(msgToken);
        }
    }

    private void respondAnswer(int msgId, int msgToken) {
        log("REQUEST_ANSWER received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        boolean answeredIncoming = mResponseGenerator.answerIncomingCall();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        log("Answered incoming call == " + answeredIncoming);
        log("REQUEST_ANSWER responded to");
    }

    private void respondHangup(int msgId, int msgToken) {
        log("REQUEST_HANGUP received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        boolean ignored = false;
        mResponseGenerator.hangupIncomingOrWaitingCall();
        // if call succOrFail == SUCCESS, then we attempt to hang up the
        // incoming/waiting call.
        log("Hung up incoming or waiting call == " + ignored);
        log("REQUEST_HANGUP responded to");
        // in Android L, unsols are sent here.
        if (mResponseGenerator.getIsAndroidL()) {
            unsolStateChangeToEnd(msgToken);
        }
    }

    private void respondHangupFgResumeBg(int msgId, int msgToken) {
        log("REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        boolean resumedBgCall = mResponseGenerator.hangupFgCallResumeBgCall();
        // if call succOrFail == SUCCESS, then we attempt to resume the
        // background call.
        log("Resume background call == " + resumedBgCall);
        log("REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND responded to");
    }

    private void respondSwitchWaitingOrHoldingAndActive(int msgId, int msgToken) {
        log("REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        boolean answeredWaiting = mResponseGenerator.answerWaitingCall();
        // if call succOrFail == SUCCESS, then we attempt to answer the
        // waiting call.
        log("Answered waiting call == " + answeredWaiting);
        log("REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE responded to");
    }

    private void respondAddParticipant(int msgId, int msgToken, byte[] msg) {
        log("REQUEST_ADD_PARTICIPANT received");
        byte[] response = null;
        boolean added = mResponseGenerator.addParticipant(msg);
        sendResponse(msgId, msgToken, response);
        log("REQUEST_ADD_PARTICIPANT responded to");
    }

    private void respondHangupWaitingOrBackground(int msgId, int msgToken) {
        log("REQUEST_HANGUP_WAITING_OR_BACKGROUND received");
        byte[] response = null;
        sendResponse(msgId, msgToken, response);
        log("REQUEST_HANGUP_WAITING_OR_BACKGROUND responded to");
    }

    private void respondHold(int msgId, int msgToken) {
        log("REQUEST_HOLD received");
        byte[] response = null;
        boolean hold = mResponseGenerator.holdActiveCall();
        sendResponse(msgId, msgToken, response);
        log("Held call == " + hold);
        log("REQUEST_HOLD responded to");
    }

    private void respondResume(int msgId, int msgToken) {
        log("REQUEST_RESUME received");
        byte[] response = null;
        boolean resumed = mResponseGenerator.resumeBgCall();
        sendResponse(msgId, msgToken, response);
        log("Resumed call == " + resumed);
        log("REQUEST_RESUME responded to");
    }

    public boolean isIncomingCallPolled() {
        return mIncomingCallPolled;
    }

    public void setIncomingCallPolled(boolean polled) {
        mIncomingCallPolled = polled;
    }

    private void respondGetCurrentCalls(int msgId, int msgToken) {
        mResponseGenerator.setIsAndroidL(false);
        log("REQUEST_GET_CURRENT_CALLS received");
        byte[] response = mResponseGenerator.generateGetCurrentCallsResponse();
        sendResponse(msgId, msgToken, response);
        if (mResponseGenerator.getDialingCall() != null) {
            unsolRingbackTone(msgToken);
            unsolStateChangeDialingToAlerting(msgToken);
        } else if (mResponseGenerator.getAlertingCall() != null) {
            unsolStateChangeAlertingToActive(msgToken);
        } else if (mResponseGenerator.getActiveCall() != null) {
            log("Call is active");
        } else if(mResponseGenerator.getIncomingCall() != null) {
            setIncomingCallPolled(true);
        }
        log("REQUEST_GET_CURRENT_CALLS responded to");
    }

    private void unsolImsNetworkStateChanged(int msgToken) {
        byte[] response = null;
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, msgToken, response);
        logDebug("Unsol ImsNetworkStateChanged");
    }

    private void unsolRadioStateChanged(int msgToken) {
        byte[] response = mResponseGenerator.generateUnsolRadioStateChanged();
        sendUnsolResponse(ImsQmiIF.UNSOL_RADIO_STATE_CHANGED, msgToken, response);
        logDebug("Unsol RadioStateChanged");
    }

    private void unsolStateChangeToDialing(int msgToken) {
        byte[] response = mResponseGenerator.generateUnsolToDialing();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        log("Call changed SET to dialing");
    }

    private void unsolStateChangeDialingToAlerting(int msgToken) {
        boolean stateChange = mResponseGenerator.callDialingToAlerting();
        byte[] response = mResponseGenerator.generateUnsolDialingToAlerting();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        log("Call changed from dialing to alerting == " + stateChange);
    }

    private void unsolStateChangeAlertingToActive(int msgToken) {
        boolean stateChange = mResponseGenerator.callAlertingToActive();
        byte[] response = mResponseGenerator.generateUnsolAlertingToActive();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        log("Call changed from alerting to active == " + stateChange);
    }

    private void unsolStateChangeToEnd(int msgToken) {
        boolean stateChange = mResponseGenerator.callActiveToEnd();
        byte[] response = mResponseGenerator.generateUnsolToEnd();
        sendUnsolResponse(ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED, msgToken,
                response);
        log("Call changed SET to ENDing == " + stateChange);
    }

    private void unsolRingbackTone(int msgToken) {
        log("Call is dialing.");
        byte[] response = mResponseGenerator.generateRingBackTone();
        sendUnsolResponse(ImsQmiIF.UNSOL_RINGBACK_TONE, msgToken, response);
    }

    private void sendResponse(int msgId, int msgToken, byte[] response) {
        int error = mResponseGenerator.getError(msgId);
        IFResponse rr = encodeMsg(msgId, msgToken, ImsQmiIF.RESPONSE, error,
                response);
        ImsSocketAgent.getInstance().sendMessage(rr);
    }

    private void sendUnsolResponse(int msgId, int msgToken, byte[] response) {
        int error = mResponseGenerator.getError(msgId);
        IFResponse rr = encodeMsg(msgId, msgToken, ImsQmiIF.UNSOL_RESPONSE, error, response);
        ImsSocketAgent.getInstance().sendMessage(rr);
    }

    public void sendMessage(IFResponse rr) {
        mSender.send(rr);
    }

    private void log(String str) {
        logDebug(str);
    }

    private void sendThread() {
        Log.d(LOGTAG, "ImsSocketAgent init sendThread");
        mSenderThread = new HandlerThread("IFMsg_Sender");
        mSenderThread.start();
        Looper looper = mSenderThread.getLooper();
        mSender = new ImsSendResponse(looper);
        log("mSender = " + mSender);
    }

    static String msgIdToString(int request) {
        // TODO - check all supported messages are covered
        switch (request) {
            case ImsQmiIF.REQUEST_GET_CURRENT_CALLS:
                return "GET_CURRENT_CALLS";
            case ImsQmiIF.REQUEST_DIAL:
                return "DIAL";
            case ImsQmiIF.REQUEST_ANSWER:
                return "REQUEST_ANSWER";
            case ImsQmiIF.REQUEST_HANGUP:
                return "HANGUP";
            case ImsQmiIF.REQUEST_HANGUP_WAITING_OR_BACKGROUND:
                return "HANGUP_WAITING_OR_BACKGROUND";
            case ImsQmiIF.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
                return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
            case ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                return "ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
            case ImsQmiIF.REQUEST_CONFERENCE:
                return "CONFERENCE";
            case ImsQmiIF.REQUEST_UDUB:
                return "UDUB";
            case ImsQmiIF.REQUEST_MODIFY_CALL_INITIATE:
                return "MODIFY_CALL_INITIATE";
            case ImsQmiIF.REQUEST_MODIFY_CALL_CONFIRM:
                return "MODIFY_CALL_CONFIRM";
            case ImsQmiIF.UNSOL_MODIFY_CALL:
                return "UNSOL_MODIFY_CALL";
            case ImsQmiIF.REQUEST_LAST_CALL_FAIL_CAUSE:
                return "LAST_CALL_FAIL_CAUSE";
            case ImsQmiIF.REQUEST_DTMF:
                return "DTMF";
            case ImsQmiIF.REQUEST_DTMF_START:
                return "DTMF_START";
            case ImsQmiIF.REQUEST_DTMF_STOP:
                return "DTMF_STOP";
            case ImsQmiIF.REQUEST_EXPLICIT_CALL_TRANSFER:
                return "ImsQmiIF.REQUEST_EXPLICIT_CALL_TRANSFER";
            case ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
                return "ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE";
            case ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE:
                return "REQUEST_IMS_REGISTRATION_STATE";
            case ImsQmiIF.REQUEST_QUERY_CLIP:
                return "REQUEST_QUERY_CLIP";
            case ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS:
                return "REQUEST_QUERY_SERVICE_STATUS";
            case ImsQmiIF.REQUEST_SET_SERVICE_STATUS:
                return "REQUEST_SET_SERVICE_STATUS";
            case ImsQmiIF.REQUEST_GET_CLIR:
                return "REQUEST_GET_CLIR";
            case ImsQmiIF.REQUEST_SET_CLIR:
                return "REQUEST_SET_CLIR";
            case ImsQmiIF.REQUEST_QUERY_CALL_FORWARD_STATUS:
                return "REQUEST_QUERY_CALL_FORWARD_STATUS";
            case ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS:
                return "REQUEST_SET_CALL_FORWARD_STATUS";
            case ImsQmiIF.REQUEST_QUERY_CALL_WAITING:
                return "REQUEST_QUERY_CALL_WAITING";
            case ImsQmiIF.REQUEST_SET_CALL_WAITING:
                return "REQUEST_SET_CALL_WAITING";
            case ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION:
                return "REQUEST_SET_SUPP_SVC_NOTIFICATION";
            case ImsQmiIF.REQUEST_SUPP_SVC_STATUS:
                return "REQUEST_SUPP_SVC_STATUS";
            case ImsQmiIF.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
                return "UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED";
            case ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED:
                return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
            case ImsQmiIF.UNSOL_CALL_RING:
                return "UNSOL_CALL_RING";
            case ImsQmiIF.UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
                return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
            case ImsQmiIF.UNSOL_RINGBACK_TONE:
                return "UNSOL_RINGBACK_TONE";
            case ImsQmiIF.UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
                return "UNSOL_EXIT_EMERGENCY_CALLBACK_MODE";
            case ImsQmiIF.REQUEST_IMS_REG_STATE_CHANGE:
                return "REQUEST_IMS_REG_STATE_CHANGE";
            case ImsQmiIF.UNSOL_RESPONSE_HANDOVER:
                return "UNSOL_RESPONSE_HANDOVER";
            case ImsQmiIF.UNSOL_REFRESH_CONF_INFO:
                return "UNSOL_REFRESH_CONF_INFO";
            case ImsQmiIF.UNSOL_SRV_STATUS_UPDATE:
                return "UNSOL_SRV_STATUS_UPDATE";
            case ImsQmiIF.UNSOL_SUPP_SVC_NOTIFICATION:
                return "UNSOL_SUPP_SVC_NOTIFICATION";
            case ImsQmiIF.UNSOL_TTY_NOTIFICATION:
                return "UNSOL_TTY_NOTIFICATION";
            case ImsQmiIF.UNSOL_RADIO_STATE_CHANGED:
                return "UNSOL_RADIO_STATE_CHANGED";
            default:
                return "<unknown message>";
        }
    }

    static String errorIdToString(int request) {
        String errorMsg;
        switch (request) {
            case ImsQmiIF.E_SUCCESS:
                errorMsg = "SUCCESS";
                break;
            case ImsQmiIF.E_RADIO_NOT_AVAILABLE:
                errorMsg = "E_RADIO_NOT_AVAILABLE";
                break;
            case ImsQmiIF.E_GENERIC_FAILURE:
                errorMsg = "E_GENERIC_FAILURE";
                break;
            case ImsQmiIF.E_REQUEST_NOT_SUPPORTED:
                errorMsg = "E_REQUEST_NOT_SUPPORTED";
                break;
            case ImsQmiIF.E_CANCELLED:
                errorMsg = "E_CANCELLED";
                break;
            case ImsQmiIF.E_UNUSED:
                errorMsg = "E_UNUSED";
                break;
            case ImsQmiIF.E_INVALID_PARAMETER:
                errorMsg = "E_INVALID_PARAMETER";
                break;
            case ImsQmiIF.E_REJECTED_BY_REMOTE:
                errorMsg = "E_REJECTED_BY_REMOTE";
                break;
            default:
                errorMsg = "E_UNKNOWN";
                break;
        }
        return errorMsg;
    }

    class ImsSendResponse extends Handler implements Runnable {
        // private LocalSocket mSocket;
        static final int EVENT_SEND = 1;
        static final String LOGTAG = "ImsSocketAgent";
        static final boolean IF_LOGD = true;
        DataOutputStream dataOutputStream = null;

        public ImsSendResponse(Looper looper) {
            super(looper);
            // mSocket = socket;
        }

        @Override
        public void run() {
            // TODO Auto-generated method stub
        }

        private void log(String str) {
            Log.d(LOGTAG, str);
        }

        public void handleMessage(Message msg) {
            // Here as of now we are not serializing the send request
            // We can serialize this send request if we need to handle
            IFResponse rr = (IFResponse) (msg.obj);
            switch (msg.what) {
                case EVENT_SEND:
                    try {
                        // This need to be synchronized if we need to handle
                        // multiple clients sockets
                        synchronized (mSocket) {
                            dataOutputStream = new DataOutputStream(mSocket.getOutputStream());
                            dataOutputStream.write(rr.mData);
                        }
                    } catch (java.io.IOException e) {
                        log("java.io.IOException when writing data to output stream");
                    }
            }
        }

        public void send(IFResponse rr) {
            Message msg;

            if (mSocket == null) {
                rr.onError(ImsQmiIF.E_RADIO_NOT_AVAILABLE, null);
                rr.release();
                return;
            }
            msg = obtainMessage(EVENT_SEND, rr);
            sendMessage(msg);
        }

    }
}
