/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
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
 *
 */

package org.codeaurora.ims;

import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.net.Socket;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.PowerManager.WakeLock;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.util.Log;

import com.android.ims.ImsException;
import com.android.ims.ImsReasonInfo;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.gsm.SuppServiceNotification;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.UUSInfo;
import com.android.internal.telephony.cdma.CdmaSmsBroadcastConfigInfo;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.google.protobuf.micro.ByteStringMicro;

/**
 * {@hide}
 */
class IFRequest {
    static final String LOG_TAG = "ImsSenderRxr";

    // ***** Class Variables
    static int sNextSerial = 0;
    static Object sSerialMonitor = new Object();
    private static Object sPoolSync = new Object();
    private static IFRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    // ***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    // FIXME delete parcel
    // Parcel mp;
    IFRequest mNext;
    byte[] mData;

    /**
     * Retrieves a new IFRequest instance from the pool.
     *
     * @param request ImsQmiIF.MsgId.REQUEST_*
     * @param result sent when operation completes
     * @return a IFRequest instance from the pool.
     */
    static IFRequest obtain(int request, Message result) {
        IFRequest rr = null;

        synchronized (sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new IFRequest();
        }

        synchronized (sSerialMonitor) {
            rr.mSerial = sNextSerial++;
        }
        rr.mRequest = request;
        rr.mResult = result;

        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }

        return rr;
    }

    /**
     * Returns a IFRequest instance to the pool. Note: This should only be
     * called once per use.
     */
    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                this.mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
            }
        }
    }

    private IFRequest() {
    }

    static void resetSerial() {
        synchronized (sSerialMonitor) {
            sNextSerial = 0;
        }
    }

    String serialString() {
        // Cheesy way to do %04d
        StringBuilder sb = new StringBuilder(8);
        String sn;

        sn = Integer.toString(mSerial);

        // sb.append("J[");
        sb.append('[');
        for (int i = 0, s = sn.length(); i < 4 - s; i++) {
            sb.append('0');
        }

        sb.append(sn);
        sb.append(']');
        return sb.toString();
    }

    void onError(int error, Object ret) {
        RuntimeException ex;
        String errorMsg;

        if (error == ImsQmiIF.E_SUCCESS) {
            ex = null;
        } else {
            errorMsg = ImsSenderRxr.errorIdToString(error);
            ex = new ImsRilException(error, errorMsg);
        }

        if (ImsSenderRxr.IF_LOGD)
            Log.d(LOG_TAG, serialString() + "< "
                    + ImsSenderRxr.msgIdToString(mRequest)
                    + " error: " + error);

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }
    }
}

/**
 * IMS implementation of the CommandsInterface. {@hide}
 */
public final class ImsSenderRxr extends ImsPhoneBaseCommands implements ImsPhoneCommandsInterface {
    static final String LOG_TAG = "ImsSenderRxr";
    static final boolean IF_LOGD = true;
    static final boolean IF_LOGV = true; // STOP SHIP if true

    static boolean sTestMode = false;

    /**
     * Wake lock timeout should be longer than the longest timeout in the vendor
     */
    private static final int DEFAULT_WAKE_LOCK_TIMEOUT = 60000;
    private static final int PDU_LENGTH_OFFSET = 4;
    private static final int MSG_TAG_LENGTH = 1;
    // ***** Instance Variables

    LocalSocket mSocket;
    HandlerThread mSenderThread;
    IFMsg_Sender mSender;
    Thread mReceiverThread;
    IFMsg_Rxr mReceiver;
    WakeLock mWakeLock;
    int mWakeLockTimeout;
    // The number of requests pending to be sent out, it increases before
    // calling
    // EVENT_SEND and decreases while handling EVENT_SEND. It gets cleared while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesPending;
    // The number of requests sent out but waiting for response. It increases
    // while
    // sending request and decreases while handling response. It should match
    // mRequestList.size() unless there are requests no replied while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesWaiting;

    /* Variable caching the Phone ID */
    private Integer mInstanceId;

    // I'd rather this be LinkedList or something
    ArrayList<IFRequest> mRequestsList = new ArrayList<IFRequest>();

    Object mLastNITZTimeInfo;

    protected RegistrantList mModifyCallRegistrants = new RegistrantList();
    protected RegistrantList mMwiRegistrants = new RegistrantList();
    protected Registrant mSsnRegistrant;

    // When we are testing emergency calls
    AtomicBoolean mTestingEmergencyCall = new AtomicBoolean(false);

    // ***** Events

    static final int EVENT_SEND = 1;
    static final int EVENT_WAKE_LOCK_TIMEOUT = 2;

    // ***** Constants

    static final int MAX_COMMAND_BYTES = (8 * 1024);
    static final int RESPONSE_SOLICITED = 0;
    static final int RESPONSE_UNSOLICITED = 1;

    static final String[] SOCKET_NAME_IF = {"qmux_radio/rild_ims0", "qmux_radio/rild_ims1", "qmux_radio/rild_ims2"};
    static final String TEST_MODE_SOCKET_NAME = "imstestrunnersocket";
    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;

    private RegistrantList mHandoverStatusRegistrants = new RegistrantList();
    private RegistrantList mRefreshConfInfoRegistrations = new RegistrantList();
    private RegistrantList mSrvStatusRegistrations = new RegistrantList();
    private RegistrantList mTtyStatusRegistrants = new RegistrantList();
    private RegistrantList mRadioStateRegistrations = new RegistrantList();

    public void registerForPhoneId(int phoneId) {
        mInstanceId = phoneId;
        disableSrvStatus(); // Disable all Ims Services
        if (mSocket != null) {
            try {
                mSocket.shutdownInput();
            } catch (IOException ex) {
                Log.i(LOG_TAG, "registerForPhoneId: " + mInstanceId + "' socket closed",
                                    ex);
            }
        }
    }

    public void registerForHandoverStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mHandoverStatusRegistrants.add(r);
    }

    public void unregisterForHandoverStatusChanged(Handler h) {
        mHandoverStatusRegistrants.remove(h);
    }

    public void registerForRefreshConfInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRefreshConfInfoRegistrations.add(r);
    }

    public void unregisterForRefreshConfInfo(Handler h) {
        mRefreshConfInfoRegistrations.remove(h);
    }

    public void registerForSrvStatusUpdate(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mSrvStatusRegistrations.add(r);
    }

    public void unregisterForSrvStatusUpdate(Handler h) {
        mSrvStatusRegistrations.remove(h);
    }

    public void registerForTtyStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mTtyStatusRegistrants.add(r);
    }

    public void unregisterForTtyStatusChanged(Handler h) {
        mTtyStatusRegistrants.remove(h);
    }

    public void setOnSuppServiceNotification(Handler h, int what, Object obj) {
        mSsnRegistrant = new Registrant (h, what, obj);
    }

    public void unSetOnSuppServiceNotification(Handler h) {
        mSsnRegistrant.clear();
    }

    class IFMsg_Sender extends Handler implements Runnable {
        public IFMsg_Sender(Looper looper) {
            super(looper);
        }

        // ***** Runnable implementation
        public void
                run() {
            // setup if needed
        }

        // ***** Handler implementation
        @Override
        public void handleMessage(Message msg) {
            IFRequest rr = (IFRequest) (msg.obj);
            IFRequest req = null;

            switch (msg.what) {
                case EVENT_SEND:
                    /**
                     * mRequestMessagePending++ already happened for every
                     * EVENT_SEND, thus we must make sure
                     * mRequestMessagePending-- happens once and only once
                     */
                    boolean alreadySubtracted = false;
                    try {
                        LocalSocket s;
                        s = mSocket;

                        if (s == null) {
                            rr.onError(ImsQmiIF.E_RADIO_NOT_AVAILABLE, null);
                            rr.release();
                            if (mRequestMessagesPending > 0)
                                mRequestMessagesPending--;
                            alreadySubtracted = true;
                            return;
                        }

                        synchronized (mRequestsList) {
                            mRequestsList.add(rr);
                            mRequestMessagesWaiting++;
                        }

                        if (mRequestMessagesPending > 0)
                            mRequestMessagesPending--;
                        alreadySubtracted = true;

                        if (rr.mData.length > MAX_COMMAND_BYTES) {
                            throw new RuntimeException(
                                    "Message is larger than max bytes allowed! "
                                            + rr.mData.length);
                        }

                        s.getOutputStream().write(rr.mData);

                    } catch (IOException ex) {
                        Log.e(LOG_TAG, "IOException", ex);
                        req = findAndRemoveRequestFromList(rr.mSerial);
                        // make sure this request has not already been handled,
                        // eg, if IFMsg_Rxr cleared the list.
                        if (req != null || !alreadySubtracted) {
                            rr.onError(ImsQmiIF.E_RADIO_NOT_AVAILABLE, null);
                            rr.release();
                        }
                    } catch (RuntimeException exc) {
                        Log.e(LOG_TAG, "Uncaught exception ", exc);
                        req = findAndRemoveRequestFromList(rr.mSerial);
                        // make sure this request has not already been handled,
                        // eg, if IFMsg_Rxr cleared the list.
                        if (req != null || !alreadySubtracted) {
                            rr.onError(ImsQmiIF.E_GENERIC_FAILURE, null);
                            rr.release();
                        }
                    } finally {
                        // Note: We are "Done" only if there are no outstanding
                        // requests or replies. Thus this code path will only
                        // release
                        // the wake lock on errors.
                        releaseWakeLockIfDone();
                    }

                    if (!alreadySubtracted && mRequestMessagesPending > 0) {
                        mRequestMessagesPending--;
                    }

                    break;

                case EVENT_WAKE_LOCK_TIMEOUT:
                    // Haven't heard back from the last request. Assume we're
                    // not getting a response and release the wake lock.
                    synchronized (mWakeLock) {
                        if (mWakeLock.isHeld()) {
                            // The timer of WAKE_LOCK_TIMEOUT is reset with each
                            // new send request. So when WAKE_LOCK_TIMEOUT
                            // occurs
                            // all requests in mRequestList already waited at
                            // least DEFAULT_WAKE_LOCK_TIMEOUT but no response.
                            // Reset mRequestMessagesWaiting to enable
                            // releaseWakeLockIfDone().
                            //
                            // Note: Keep mRequestList so that delayed response
                            // can still be handled when response finally comes.
                            if (mRequestMessagesWaiting != 0) {
                                Log.d(LOG_TAG, "NOTE: mReqWaiting is NOT 0 but"
                                        + mRequestMessagesWaiting + " at TIMEOUT, reset!"
                                        + " There still msg waitng for response");

                                mRequestMessagesWaiting = 0;

                                if (IF_LOGD) {
                                    synchronized (mRequestsList) {
                                        int count = mRequestsList.size();
                                        Log.d(LOG_TAG, "WAKE_LOCK_TIMEOUT " +
                                                " mRequestList=" + count);

                                        for (int i = 0; i < count; i++) {
                                            rr = mRequestsList.get(i);
                                            Log.d(LOG_TAG, i + ": [" + rr.mSerial + "] "
                                                    + msgIdToString(rr.mRequest));
                                        }
                                    }
                                }
                            }
                            // mRequestMessagesPending shows how many
                            // requests are waiting to be sent (and before
                            // to be added in request list) since star the
                            // WAKE_LOCK_TIMEOUT timer. Since WAKE_LOCK_TIMEOUT
                            // is the expected time to get response, all
                            // requests
                            // should already sent out (i.e.
                            // mRequestMessagesPending is 0 )while TIMEOUT
                            // occurs.
                            if (mRequestMessagesPending != 0) {
                                Log.e(LOG_TAG, "ERROR: mReqPending is NOT 0 but"
                                        + mRequestMessagesPending + " at TIMEOUT, reset!");
                                mRequestMessagesPending = 0;

                            }
                            mWakeLock.release();
                        }
                    }
                    break;
            }
        }
    }

    /**
     * Reads in a single message off the wire. A message consists of a 4-byte
     * little-endian length and a subsequent series of bytes. The final message
     * (length header omitted) is read into <code>buffer</code> and the length
     * of the final message (less header) is returned. A return value of -1
     * indicates end-of-stream.
     *
     * @param is non-null; Stream to read from
     * @param buffer Buffer to fill in. Must be as large as maximum message
     *            size, or an ArrayOutOfBounds exception will be thrown.
     * @return Length of message less header, or -1 on end of stream.
     * @throws IOException
     */
    private static int readMessage(InputStream is, byte[] buffer)
            throws IOException {
        int countRead;
        int offset;
        int remaining;
        int messageLength;

        // First, read in the length of the message
        offset = 0;
        remaining = 4;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0) {
                Log.e(LOG_TAG, "Hit EOS reading message length");
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        messageLength = ((buffer[0] & 0xff) << 24)
                | ((buffer[1] & 0xff) << 16)
                | ((buffer[2] & 0xff) << 8)
                | (buffer[3] & 0xff);

        // Then, re-use the buffer and read in the message itself
        offset = 0;
        remaining = messageLength;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0) {
                Log.e(LOG_TAG, "Hit EOS reading message.  messageLength=" + messageLength
                        + " remaining=" + remaining);
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        return messageLength;
    }

    class IFMsg_Rxr implements Runnable {

        byte[] buffer;
        IFMsg_Rxr() {
            buffer = new byte[MAX_COMMAND_BYTES];
        }

        public void run() {
            int retryCount = 0;
            boolean killMe = false;

            String ifSocket = SOCKET_NAME_IF[mInstanceId];

            try {
                if (!killMe) {
                    for (;;) {
                        LocalSocket s = null;
                        LocalSocketAddress l;

                        /* Depending on the Phone ID, select the socket name */
                        ifSocket = SOCKET_NAME_IF[mInstanceId];

                        try {
                             s = new LocalSocket();
                             if (sTestMode) {
                                 l = new LocalSocketAddress(TEST_MODE_SOCKET_NAME);
                             } else {
                                 l = new LocalSocketAddress(ifSocket,
                                         LocalSocketAddress.Namespace.RESERVED);
                             }
                             s.connect(l);
                             Log.d(LOG_TAG, "Connecting to socket " + s);

                        } catch (IOException ex) {
                            Log.e(LOG_TAG,
                                    "Exception in socket create'" + ex.toString());
                            // don't print an error message after the the first time
                            // or after the 8th time

                            if (retryCount == 8) {
                                Log.e(LOG_TAG,
                                        "Couldn't find " + ifSocket + "socket after " + retryCount
                                                + " times, continuing to retry silently");
                                disableSrvStatus(); // Disable all Ims Services
                                try {
                                    s.close();
                                    killMe = true;
                                    return;
                                } catch (IOException e) {
                                    Log.e(LOG_TAG, "IOException", ex);
                                    return;
                                }
                            } else if (retryCount > 0 && retryCount < 8) {
                                Log.i(LOG_TAG,
                                        "Couldn't find " + ifSocket + "socket; retrying after timeout");
                            }

                            try {
                                Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
                            } catch (InterruptedException er) {
                            }

                            retryCount++;
                            continue;
                        }

                        retryCount = 0;

                        mSocket = s;

                        Log.i(LOG_TAG, "Connected to '" + mSocket + "' socket");
                        Log.i(LOG_TAG, "For instance [" + mInstanceId + "] connected to '" + ifSocket + "' socket");

                        int length = 0;
                        try {
                            InputStream is = mSocket.getInputStream();

                            for (;;) {

                                length = readMessage(is, buffer);

                                if (length < 0) {
                                    // End-of-stream reached
                                    break;
                                }

                                Log.v(LOG_TAG, "Read packet: " + length + " bytes");

                                if (length > 0) processResponse(buffer, length);

                            }
                        } catch (java.io.IOException ex) {
                            Log.i(LOG_TAG, "'" + ifSocket + "' socket closed",
                                    ex);
                        } catch (Throwable tr) {
                            Log.e(LOG_TAG, "Uncaught exception read length=" + length +
                                    "Exception:" + tr.toString());
                        }

                        Log.i(LOG_TAG, "Disconnected from " + ifSocket + " socket");

                        if (mSocket != null) {
                            try {
                                mSocket.close();
                            } catch (IOException ex) {
                            }
                            mSocket = null;
                        }
                        IFRequest.resetSerial();

                        // Clear request list on close

                        clearRequestsList(ImsQmiIF.E_RADIO_NOT_AVAILABLE, false);
                    }
                }
            } catch (Throwable tr) {
                Log.e(LOG_TAG, "Uncaught exception", tr);
            }

            /* We're disconnected so we don't know the version */
            // notifyRegistrantsIFConnectionChanged(-1);
        }
    }

    // ***** Constructors

    public ImsSenderRxr(Context context)
    {
        super(context);

        mPhoneType = 0; // NO_PHONE;
        mInstanceId = 0; // default socket

        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOG_TAG);
        mWakeLock.setReferenceCounted(false);
        mWakeLockTimeout = SystemProperties.getInt(TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT,
                DEFAULT_WAKE_LOCK_TIMEOUT);
        mRequestMessagesPending = 0;
        mRequestMessagesWaiting = 0;
        sTestMode = SystemProperties.getBoolean("persist.qualcomm.imstestrunner", false) &&
                Build.IS_DEBUGGABLE;

        mSenderThread = new HandlerThread("IFMsg_Sender");
        mSenderThread.start();

        Looper looper = mSenderThread.getLooper();
        mSender = new IFMsg_Sender(looper);

        log("Starting IFMsg_Rxr");
        mReceiver = new IFMsg_Rxr();
        mReceiverThread = new Thread(mReceiver, "IFMsg_Rxr");
        mReceiverThread.start();

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        // context.registerReceiver(mIntentReceiver, filter);
    }

    /**
     * Holds a PARTIAL_WAKE_LOCK whenever a) There is outstanding request sent
     * to the interface and no replied b) There is a request pending to be sent
     * out. There is a WAKE_LOCK_TIMEOUT to release the lock, though it
     * shouldn't happen often.
     */

    private void acquireWakeLock() {
        synchronized (mWakeLock) {
            mWakeLock.acquire();
            mRequestMessagesPending++;

            mSender.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
            Message msg = mSender.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
            mSender.sendMessageDelayed(msg, mWakeLockTimeout);
        }
    }

    private void releaseWakeLockIfDone() {
        synchronized (mWakeLock) {
            if (mWakeLock.isHeld() &&
                    (mRequestMessagesPending == 0) &&
                    (mRequestMessagesWaiting == 0)) {
                mSender.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
                mWakeLock.release();
            }
        }
    }

    public void send(IFRequest rr) {
        Message msg;

        if (mSocket == null) {
            rr.onError(ImsQmiIF.E_RADIO_NOT_AVAILABLE, null);
            rr.release();
            return;
        }

        msg = mSender.obtainMessage(EVENT_SEND, rr);

        acquireWakeLock();

        msg.sendToTarget();
    }

    public void processResponse(byte[] data, int length) {

        // Parse tag length from first one byte
        int msglen = 1;
        int startIndex = 0;
        int endIndex = startIndex + msglen;
        byte[] msg = null;

        logv("processResponse");
        if (ImsSenderRxr.IF_LOGV) {
            if (length > 0) {
                log("Response data: " + Arrays.toString(Arrays.copyOfRange(data, 0, length)));
            }
        }

        if (endIndex <= length) {
            msglen = data[startIndex];

            // Parse tag
            ImsQmiIF.MsgTag tag = null;
            startIndex = endIndex;
            endIndex = startIndex + msglen;

            if ((endIndex <= length) && (msglen > 0)) {

                msg = new byte[msglen];

                try {
                    System.arraycopy(data, startIndex, msg, 0, msglen);
                    // Convert tag in bytes to local data structure
                    tag = ImsQmiIF.MsgTag.parseFrom(msg);
                } catch (IndexOutOfBoundsException ex) {
                    log(" IndexOutOfBoundsException while parsing msg tag");
                    releaseWakeLockIfDone();
                    return;
                } catch (ArrayStoreException ex) {
                    log(" ArrayStoreException while parsing msg tag");
                    releaseWakeLockIfDone();
                    return;
                } catch (NullPointerException ex) {
                    log(" NullPointerException while parsing msg tag");
                    releaseWakeLockIfDone();
                    return;
                } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                    log("InvalidProtocolBufferException while parsing msg tag");
                    releaseWakeLockIfDone();
                    return;
                }
                log(" Tag " + tag.getToken() + " " + tag.getType() + " "
                        + tag.getId() + " " + tag.getError());

                // Parse message content if present
                startIndex = endIndex;
                msglen = length - startIndex;
                endIndex = startIndex + msglen;

                msg = null;
                if ((endIndex <= length) && (msglen > 0)) {
                    msg = new byte[msglen];
                    try {
                        System.arraycopy(data, startIndex, msg, 0, msglen);
                    } catch (IndexOutOfBoundsException ex) {
                        log(" IndexOutOfBoundsException while parsing msg tag");
                    } catch (ArrayStoreException ex) {
                        log(" ArrayStoreException while parsing msg tag");
                    } catch (NullPointerException ex) {
                        log(" NullPointerException while parsing msg tag");
                    }

                }

                // Call message handler based on message type present in tag
                switch (tag.getType()) {
                    case ImsQmiIF.UNSOL_RESPONSE:
                        processUnsolicited(tag, msg);
                        break;
                    case ImsQmiIF.RESPONSE:
                        processSolicited(tag, msg);
                        break;
                    default:
                        log(" Unknown Message Type  ");
                        break;
                }
            }
            else {
                log("Error in parsing msg tag");
            }
        } else {
            log("Error in parsing msg tag length");
        }
        releaseWakeLockIfDone();
    }

    /**
     * Release each request in mReqeustsList then clear the list
     *
     * @param error is the ImsQmiIF.Error sent back
     * @param loggable true means to print all requests in mRequestslist
     */
    private void clearRequestsList(int error, boolean loggable) {
        IFRequest rr;
        synchronized (mRequestsList) {
            int count = mRequestsList.size();
            if (IF_LOGD && loggable) {
                Log.d(LOG_TAG, "WAKE_LOCK_TIMEOUT " +
                        " mReqPending=" + mRequestMessagesPending +
                        " mRequestList=" + count);
            }

            for (int i = 0; i < count; i++) {
                rr = mRequestsList.get(i);
                if (IF_LOGD && loggable) {
                    Log.d(LOG_TAG, i + ": [" + rr.mSerial + "] " +
                            msgIdToString(rr.mRequest));
                }
                rr.onError(error, null);
                rr.release();
            }
            mRequestsList.clear();
            mRequestMessagesWaiting = 0;
        }
    }

    private IFRequest findAndRemoveRequestFromList(int serial) {
        synchronized (mRequestsList) {
            for (int i = 0, s = mRequestsList.size(); i < s; i++) {
                IFRequest rr = mRequestsList.get(i);

                if (rr.mSerial == serial) {
                    mRequestsList.remove(i);
                    if (mRequestMessagesWaiting > 0)
                        mRequestMessagesWaiting--;
                    return rr;
                }
            }
        }

        return null;
    }

    protected void processSolicited(ImsQmiIF.MsgTag tag, byte[] message) {
        boolean found = false;

        int serial = tag.getToken();
        int error = tag.getError();
        int id = tag.getId();

        IFRequest rr;

        rr = findAndRemoveRequestFromList(serial);

        if (rr == null) {
            Log.w(LOG_TAG, "Unexpected solicited response! sn: "
                    + serial + " error: " + error);
            return;
        }

        Object ret = null;

        if (error == ImsQmiIF.E_SUCCESS || ((message != null) && (message.length >= 1))) {
            // either command succeeds or command fails but with data payload
            try {
                switch (id) {
                    case ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE:
                        ret = responseImsRegistration(message);
                        break;
                    case ImsQmiIF.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: {
                        if (mTestingEmergencyCall.getAndSet(false)) {
                            if (mEmergencyCallbackModeRegistrant != null) {
                                log("testing emergency call, notify ECM Registrants");
                                mEmergencyCallbackModeRegistrant.notifyRegistrant();
                            }
                        }
                        break;
                    }
                    case ImsQmiIF.REQUEST_GET_CURRENT_CALLS:
                        ret = responseCallList(message);
                        break;
                    case ImsQmiIF.REQUEST_LAST_CALL_FAIL_CAUSE:
                        ret = responseCallFailCause(message);
                        break;
                    case ImsQmiIF.REQUEST_GET_CLIR:
                        ret = responseGetClir(message);
                        break;
                    case ImsQmiIF.REQUEST_QUERY_CALL_FORWARD_STATUS:
                        ret = responseQueryCallForward(message);
                        break;
                    case ImsQmiIF.REQUEST_QUERY_CALL_WAITING:
                        ret = responseQueryCallWaiting(message);
                        break;
                    case ImsQmiIF.REQUEST_QUERY_CLIP:
                        ret = responseQueryClip(message);
                        break;
                    case ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS:
                        ret = handleSrvStatus(message);
                        break;
                    case ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS:
                    case ImsQmiIF.REQUEST_SET_CALL_WAITING:
                    case ImsQmiIF.REQUEST_SUPP_SVC_STATUS:
                        ret = responseSuppSvcStatus(message);
                        break;
                    case ImsQmiIF.REQUEST_QUERY_VT_CALL_QUALITY:
                        ret = responseQueryVideoCallQuality(message);
                        break;
                    case ImsQmiIF.REQUEST_GET_RTP_STATISTICS:
                        ret = responseGetRtpStatistics(message);
                        break;
                    case ImsQmiIF.REQUEST_GET_RTP_ERROR_STATISTICS:
                        ret = responseGetRtpErrorStatistics(message);
                        break;
                    case ImsQmiIF.REQUEST_GET_COLR:
                        ret = responseGetColr(message);
                        break;
                    case ImsQmiIF.REQUEST_GET_WIFI_CALLING_STATUS:
                        ret = responseGetWifiCallingStatus(message);
                        break;
                    case ImsQmiIF.REQUEST_SET_SERVICE_STATUS:
                    case ImsQmiIF.REQUEST_DIAL:
                    case ImsQmiIF.REQUEST_SEND_UI_TTY_MODE:
                    case ImsQmiIF.REQUEST_ANSWER:
                    case ImsQmiIF.REQUEST_DEFLECT_CALL:
                    case ImsQmiIF.REQUEST_ADD_PARTICIPANT:
                    case ImsQmiIF.REQUEST_HANGUP:
                    case ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                    case ImsQmiIF.REQUEST_HOLD:
                    case ImsQmiIF.REQUEST_RESUME:
                    case ImsQmiIF.REQUEST_CONFERENCE:
                    case ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
                    case ImsQmiIF.REQUEST_DTMF:
                    case ImsQmiIF.REQUEST_DTMF_START:
                    case ImsQmiIF.REQUEST_DTMF_STOP:
                    case ImsQmiIF.REQUEST_HANGUP_WAITING_OR_BACKGROUND:
                    case ImsQmiIF.REQUEST_MODIFY_CALL_INITIATE:
                    case ImsQmiIF.REQUEST_MODIFY_CALL_CONFIRM:
                    case ImsQmiIF.REQUEST_SET_CLIR:
                    case ImsQmiIF.REQUEST_IMS_REG_STATE_CHANGE:
                    case ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION:
                    case ImsQmiIF.REQUEST_SET_VT_CALL_QUALITY:
                    case ImsQmiIF.REQUEST_SET_COLR:
                    case ImsQmiIF.REQUEST_SET_WIFI_CALLING_STATUS:
                        // no data
                        break;
                    default:
                        throw new RuntimeException("Unrecognized solicited response: "
                                + rr.mRequest);
                }
            } catch (Throwable tr) {
                // Exceptions here usually mean invalid responses

                Log.w(LOG_TAG, rr.serialString() + "< "
                        + msgIdToString(rr.mRequest) + "[SUB" + mInstanceId + "]"
                        + " exception, possible invalid response", tr);

                if (rr.mResult != null) {
                    AsyncResult.forMessage(rr.mResult, null, tr);
                    rr.mResult.sendToTarget();
                }
                rr.release();
                return;
            }
        }
        if (error != ImsQmiIF.E_SUCCESS) {
            rr.onError(error, ret);
            rr.release();
            return;
        }

        if (IF_LOGD)
            log(rr.serialString() + "< " + msgIdToString(rr.mRequest)
                    + " " + retToString(rr.mRequest, ret));

        if (rr.mResult != null) {
            AsyncResult.forMessage(rr.mResult, ret, null);
            rr.mResult.sendToTarget();
        }

        rr.release();
    }

    private String retToString(int req, Object ret) {

        if (ret == null)
            return "";

        StringBuilder sb;
        String s;
        int length;
        if (ret instanceof int[]) {
            int[] intArray = (int[]) ret;
            length = intArray.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(intArray[i++]);
                while (i < length) {
                    sb.append(", ").append(intArray[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (ret instanceof String[]) {
            String[] strings = (String[]) ret;
            length = strings.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(strings[i++]);
                while (i < length) {
                    sb.append(", ").append(strings[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (req == ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED
                || req == ImsQmiIF.REQUEST_GET_CURRENT_CALLS ) {
            ArrayList<DriverCallIms> calls = (ArrayList<DriverCallIms>) ret;
            sb = new StringBuilder(" ");
            for (DriverCallIms dc : calls) {
                sb.append("[").append(dc).append("] ");
            }
            s = sb.toString();
        } else {
            s = ret.toString();
        }
        return s;
    }

    public void registerForModifyCall(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mModifyCallRegistrants.add(r);
    }

    public void unregisterForModifyCall(Handler h) {
        mModifyCallRegistrants.remove(h);
    }

    public void registerForMwi(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mMwiRegistrants.add(r);
    }

    public void unregisterForMwi(Handler h) {
        mMwiRegistrants.remove(h);
    }

    protected void processUnsolicited(ImsQmiIF.MsgTag tag, byte[] message) {
        int response = tag.getId();
        Object ret = null;

        try {
            switch (response) {
                case ImsQmiIF.UNSOL_RINGBACK_TONE:
                    if (message != null) ret = responseCallRingBack(message);
                    break;
                case ImsQmiIF.UNSOL_CALL_RING:
                case ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED:
                    if (message != null) ret = responseCallList(message);
                    break;
                case ImsQmiIF.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
                    if (message != null) ret = responseImsRegistration(message);
                    break;
                case ImsQmiIF.UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
                case ImsQmiIF.UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
                    ret = null;
                    break;
                case ImsQmiIF.UNSOL_MODIFY_CALL:
                    if (message != null) ret = responseModifyCall(message);
                    if (IF_LOGD) unsljLogRet(response, ret);
                    mModifyCallRegistrants
                            .notifyRegistrants(new AsyncResult(null, ret, null));
                    break;
                case ImsQmiIF.UNSOL_RESPONSE_HANDOVER:
                    if (message != null) ret = responseHandover(message);
                    break;
                case ImsQmiIF.UNSOL_REFRESH_CONF_INFO:
                    if (message != null) ret = handleRefreshInfo(message);
                    break;
                case ImsQmiIF.UNSOL_SRV_STATUS_UPDATE:
                    if (message != null) ret = handleSrvStatus(message);
                    break;
                case ImsQmiIF.UNSOL_TTY_NOTIFICATION:
                    if (message != null) ret = handleTtyNotify(message);
                    break;
                case ImsQmiIF.UNSOL_SUPP_SVC_NOTIFICATION:
                    if (message != null) ret = responseSuppServiceNotification(message);
                    break;
                case ImsQmiIF.UNSOL_RADIO_STATE_CHANGED:
                    if (message != null) ret = handleRadioStateChange(message);
                    break;
                case ImsQmiIF.UNSOL_MWI:
                    if (message != null) ret = handleMwi(message);
                    break;
                default:
                    throw new RuntimeException("Unrecognized unsol response: " + response);
            }
        } catch (Throwable tr) {
            Log.e(LOG_TAG, "Exception processing unsol response: " + response +
                    "Exception:" + tr.toString());
            return;
        }

        switch (response) {
            case ImsQmiIF.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
                if (IF_LOGD)
                    unsljLog(response);

                mImsNetworkStateChangedRegistrants
                        .notifyRegistrants(new AsyncResult(null, ret, null));
                break;
            case ImsQmiIF.UNSOL_RESPONSE_CALL_STATE_CHANGED:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                mCallStateRegistrants
                        .notifyRegistrants(new AsyncResult(null, ret, null));
                break;
            case ImsQmiIF.UNSOL_CALL_RING:
                if (IF_LOGD)
                    unsljLogRet(response, ret);

                if (mRingRegistrant != null) {
                    mRingRegistrant.notifyRegistrant(
                            new AsyncResult(null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
                if (IF_LOGD)
                    unsljLog(response);

                if (mEmergencyCallbackModeRegistrant != null) {
                    mEmergencyCallbackModeRegistrant.notifyRegistrant();
                }
                break;

            case ImsQmiIF.UNSOL_RINGBACK_TONE:
                boolean playtone = false;
                if (IF_LOGD)
                    unsljLogvRet(response, ret);
                if (ret != null) playtone = (((int[]) ret)[0] == 1);
                if (mRingbackToneRegistrants != null) {
                    mRingbackToneRegistrants.notifyRegistrants(
                            new AsyncResult(null, playtone, null));
                }
                break;
            case ImsQmiIF.UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
                if (IF_LOGD)
                    unsljLogRet(response, ret);

                if (mExitEmergencyCallbackModeRegistrants != null) {
                    mExitEmergencyCallbackModeRegistrants.notifyRegistrants(
                            new AsyncResult(null, null, null));
                }
                break;
            case ImsQmiIF.UNSOL_RESPONSE_HANDOVER:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (ret != null) {
                    mHandoverStatusRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_REFRESH_CONF_INFO:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (ret != null) {
                    mRefreshConfInfoRegistrations
                    .notifyRegistrants(new AsyncResult(null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_SRV_STATUS_UPDATE:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (ret != null) {
                    mSrvStatusRegistrations
                    .notifyRegistrants(new AsyncResult(null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_TTY_NOTIFICATION:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (ret != null) {
                    mTtyStatusRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_SUPP_SVC_NOTIFICATION:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (mSsnRegistrant != null) {
                    mSsnRegistrant.notifyRegistrant(
                                        new AsyncResult (null, ret, null));
                }
                break;
            case ImsQmiIF.UNSOL_MWI:
                if (IF_LOGD)
                    unsljLogRet(response, ret);
                if (ret != null) {
                    mMwiRegistrants.notifyRegistrants(
                            new AsyncResult (null, ret, null));
                }
                break;
        }
    }

    private Object responseModifyCall(byte[] message) {
        CallModify callModify = new CallModify();
        try {
            ImsQmiIF.CallModify callModifyIF = ImsQmiIF.CallModify.parseFrom(message);
            callModify.call_details.call_type = callModifyIF.getCallDetails().getCallType();
            callModify.call_details.call_domain = callModifyIF.getCallDetails().getCallDomain();
            callModify.call_details.callsubstate = callModifyIF.getCallDetails().getCallSubstate();
            callModify.call_details.callMediaId = callModifyIF.getCallDetails().getMediaId();
            List<String> extrasList = callModifyIF.getCallDetails().getExtrasList();
            callModify.call_details.extras = extrasList.toArray(new String[extrasList.size()]);
            callModify.call_index = callModifyIF.getCallIndex();
            callModify.error = callModifyIF.hasError() ? callModifyIF.getError()
                    : ImsQmiIF.E_SUCCESS;
            log("responseModifyCall " + callModify);
        } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
            log(" Message parsing. InvalidProtocolBufferException ");
        }
        return callModify;
    }

    private Object responseQueryVideoCallQuality(byte[] message) {
        if (message == null) {
            log("responseQueryVideoCallQuality failed, message is null");
        } else {
            try {
                ImsQmiIF.VideoCallQuality vQuality = ImsQmiIF.VideoCallQuality.parseFrom(message);
                if (vQuality.hasQuality()) {
                    int quality = vQuality.getQuality();
                    log("responseQueryVideoCallQuality, quality=" + quality);
                    return quality;
                } else {
                    log("responseQueryVideoCallQuality failed. Quality is not set.");
                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log(" Message parsing. InvalidProtocolBufferException ");
            }
        }
        return null;
    }

    private Object responseGetRtpStatistics(byte[] message) {
        if (message == null) {
            log("responseGetRtpStatistics failed, message is null");
        } else {
            try {
                ImsQmiIF.RtpStatisticsData rtpStatData =
                        ImsQmiIF.RtpStatisticsData.parseFrom(message);
                if (rtpStatData.hasCount()) {
                    long packetCount = rtpStatData.getCount();
                    log("responseGetRtpStatistics, packetCount = " + packetCount);
                    return packetCount;
                } else {
                    log("responseGetRtpStatistics failed");
                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log(" Message parsing. InvalidProtocolBufferException ");
            }
        }
        return null;
    }

    private Object responseGetRtpErrorStatistics(byte[] message) {
        if (message == null) {
            log("responseGetRtpErrorStatistics failed, message is null");
        } else {
            try {
                ImsQmiIF.RtpStatisticsData rtpStatData =
                        ImsQmiIF.RtpStatisticsData.parseFrom(message);
                if (rtpStatData.hasCount()) {
                    long perCount = rtpStatData.getCount();
                    log("responseGetRtpErrorStatistics, perCount = " + perCount);
                    return perCount;
                } else {
                    log("responseGetRtpErrorStatistics failed");
                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log(" Message parsing. InvalidProtocolBufferException ");
            }
        }
        return null;
    }

    private Object responseGetWifiCallingStatus(byte[] message) {
        ImsQmiIF.WifiCallingInfo wifiCallingInfo = null;
        if (message != null && message.length >= 1) {
            try {
                wifiCallingInfo = ImsQmiIF.WifiCallingInfo.parseFrom(message);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log(" Message parsing. InvalidProtocolBufferException ");
            }
        } else {
            log("responseGetWifiCallingStatus failed, message is null");
        }
        return wifiCallingInfo;
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

    static String msgIdToString(int request) {
        // TODO - check all supported messages are covered
        switch (request) {
            case ImsQmiIF.REQUEST_GET_CURRENT_CALLS:
                return "GET_CURRENT_CALLS";
            case ImsQmiIF.REQUEST_DIAL:
                return "DIAL";
            case ImsQmiIF.REQUEST_ANSWER:
                return "REQUEST_ANSWER";
            case ImsQmiIF.REQUEST_DEFLECT_CALL:
                return "REQUEST_DEFLECT_CALL";
            case ImsQmiIF.REQUEST_ADD_PARTICIPANT:
                return "REQUEST_ADD_PARTICIPANT";
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
            case ImsQmiIF.REQUEST_SEND_UI_TTY_MODE:
                return "REQUEST_SEND_UI_TTY_MODE";
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
            case ImsQmiIF.REQUEST_QUERY_VT_CALL_QUALITY:
                return "REQUEST_QUERY_VT_CALL_QUALITY";
            case ImsQmiIF.REQUEST_GET_RTP_STATISTICS:
                return "REQUEST_GET_RTP_STATISTICS";
            case ImsQmiIF.REQUEST_GET_RTP_ERROR_STATISTICS:
                return "REQUEST_GET_RTP_ERROR_STATISTICS";
            case ImsQmiIF.REQUEST_SET_VT_CALL_QUALITY:
                return "REQUEST_SET_VT_CALL_QUALITY";
            case ImsQmiIF.REQUEST_GET_WIFI_CALLING_STATUS:
                return "REQUEST_GET_WIFI_CALLING_STATUS";
            case ImsQmiIF.REQUEST_SET_WIFI_CALLING_STATUS:
                return "REQUEST_SET_WIFI_CALLING_STATUS";
            case ImsQmiIF.REQUEST_GET_COLR:
                return "REQUEST_GET_COLR";
            case ImsQmiIF.REQUEST_SET_COLR:
                return "REQUEST_SET_COLR";
            case ImsQmiIF.REQUEST_HOLD:
                return "REQUEST_HOLD";
            case ImsQmiIF.REQUEST_RESUME:
                return "REQUEST_RESUME";
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
            case ImsQmiIF.UNSOL_MWI:
                return "UNSOL_MWI";
            default:
                return "<unknown message>";
        }
    }

    public void log(String msg) {
        Log.d(LOG_TAG, msg + "[SUB" + mInstanceId + "]");
    }

    public void logv(String msg) {
        if (ImsSenderRxr.IF_LOGV) {
            Log.v(LOG_TAG, msg + "[SUB" + mInstanceId + "]");
        }
    }

    /**
     * Use this only for unimplemented methods. Prints stack trace if the
     * unimplemented method is ever called
     */
    public void logUnimplemented() {
        try {
            Exception e = new Exception();
            throw e;
        } catch (Exception e) {
            Log.d(LOG_TAG, "Unimplemented method. Stack trace: ");
            e.printStackTrace();
        }
    }

    public void unsljLog(int response) {
        log("[UNSL]< " + msgIdToString(response));
    }

    public void unsljLogMore(int response, String more) {
        log("[UNSL]< " + msgIdToString(response) + " " + more);
    }

    public void unsljLogRet(int response, Object ret) {
        log("[UNSL]< " + msgIdToString(response) + " " + retToString(response, ret));
    }

    public void unsljLogvRet(int response, Object ret) {
        logv("[UNSL]< " + msgIdToString(response) + " " + retToString(response, ret));
    }

    @Override
    public void setPhoneType(int phoneType) { // Called by Phone constructor
        if (IF_LOGD)
            log("setPhoneType=" + phoneType + " old value=" + mPhoneType);
        mPhoneType = phoneType;
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("IMS INTERFACE:");
        pw.println(" mSocket=" + mSocket);
        pw.println(" mSenderThread=" + mSenderThread);
        pw.println(" mSender=" + mSender);
        pw.println(" mReceiverThread=" + mReceiverThread);
        pw.println(" mReceiver=" + mReceiver);
        pw.println(" mWakeLock=" + mWakeLock);
        pw.println(" mWakeLockTimeout=" + mWakeLockTimeout);
        synchronized (mRequestsList) {
            pw.println(" mRequestMessagesPending=" + mRequestMessagesPending);
            pw.println(" mRequestMessagesWaiting=" + mRequestMessagesWaiting);
            int count = mRequestsList.size();
            pw.println(" mRequestList count=" + count);
            for (int i = 0; i < count; i++) {
                IFRequest rr = mRequestsList.get(i);
                pw.println("  [" + rr.mSerial + "] " + msgIdToString(rr.mRequest));
            }
        }
        pw.println(" mLastNITZTimeInfo=" + mLastNITZTimeInfo);
        pw.println(" mTestingEmergencyCall=" + mTestingEmergencyCall.get());
    }

    /** Message tag encoding */
    public void encodeMsg(int id, Message result, byte[] msg) {

        int msgLen = 0;
        int index = 0;
        int totalPacketLen = 0;
        IFRequest rr = IFRequest.obtain(id, result);

        ImsQmiIF.MsgTag msgtag;
        msgtag = new ImsQmiIF.MsgTag();
        msgtag.setToken(rr.mSerial);
        msgtag.setType(ImsQmiIF.REQUEST);
        msgtag.setId(rr.mRequest);
        msgtag.setError(ImsQmiIF.E_SUCCESS);
        byte[] tagb = msgtag.toByteArray();

        if (msg != null) {
            msgLen = msg.length;
        }

        // data is the byte stream that will be sent across the socket
        // byte 0..3 = total msg length (i.e. tag length + msglen)
        // byte 4 = msgtag length
        // byte 5..tagb.length = msgtag in bytes
        // byte 5+tagb.lenght..msglen = msg in bytes
        totalPacketLen = PDU_LENGTH_OFFSET + tagb.length + msgLen + MSG_TAG_LENGTH;
        rr.mData = new byte[totalPacketLen];

        // length in big endian
        rr.mData[index++] = rr.mData[index++] = 0;
        rr.mData[index++] = (byte)(((totalPacketLen - PDU_LENGTH_OFFSET) >> 8) & 0xff);
        rr.mData[index++] = (byte)(((totalPacketLen - PDU_LENGTH_OFFSET)) & 0xff);

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
            log("Message data: " + Arrays.toString(rr.mData));
        }

        send(rr);
        return;
    }

    private byte[] processDial(String address, int clirMode, CallDetails callDetails) {
        boolean isConferenceUri = false;
        if (callDetails != null && callDetails.extras != null) {
            String value = callDetails.getValueForKeyFromExtras(callDetails.extras,
                    CallDetails.EXTRAS_IS_CONFERENCE_URI);
            if (value != null && Boolean.valueOf(value)) {
                isConferenceUri = true;
            }
        }

        logv("process dial isConfererenceUri = " + isConferenceUri);
        ImsQmiIF.CallDetails callDetailsIF = new ImsQmiIF.CallDetails();
        if (callDetails != null) {
            callDetailsIF.setCallDomain(callDetails.call_domain);
            callDetailsIF.setCallType(callDetails.call_type);
            // Extract all extras from CallDetails.
            if (callDetails.extras != null) {
                for (int i = 0; i < callDetails.extras.length; i++) {
                    callDetailsIF.addExtras(callDetails.extras[i]);
                    logv("Packing extra string '" + callDetails.extras[i] + "'");
                }
            }
        }


        ImsQmiIF.Dial dialIF = new ImsQmiIF.Dial();
        dialIF.setAddress(address);
        dialIF.setCallDetails(callDetailsIF);
        dialIF.setClir(clirMode);
        switch(clirMode){
            case CommandsInterface.CLIR_SUPPRESSION:
                dialIF.setPresentation(ImsQmiIF.IP_PRESENTATION_NUM_RESTRICTED);
                break;
            case CommandsInterface.CLIR_INVOCATION:
            case CommandsInterface.CLIR_DEFAULT:
            default:
                dialIF.setPresentation(ImsQmiIF.IP_PRESENTATION_NUM_ALLOWED);
                break;
        }

        if (isConferenceUri) {
            dialIF.setIsConferenceUri(isConferenceUri);
        }

        byte[] dialb = dialIF.toByteArray();
        return dialb;
    }

    public void addParticipant(String address, int clirMode, CallDetails callDetails,
            Message result) {
        logv("addParticipant address= " + address + "clirMode= " + clirMode
                + " callDetails= " + callDetails);
        byte[] dialb = processDial(address, clirMode, callDetails);

        encodeMsg(ImsQmiIF.REQUEST_ADD_PARTICIPANT, result, dialb);
    }

    public void
    dial(String address, int clirMode, CallDetails callDetails,
            Message result) {
        logv("dial address= " + address + "clirMode= " + clirMode
                + " callDetails= " + callDetails);
        byte[] dialb = processDial(address, clirMode, callDetails);

        encodeMsg(ImsQmiIF.REQUEST_DIAL, result, dialb);
    }

    public void
    acceptCall(Message result, int callType) {
        logv("acceptCall callType=" + callType);
        int callTypeIF = callType;
        ImsQmiIF.Answer answerIF = new ImsQmiIF.Answer();
        answerIF.setCallType(callTypeIF);
        byte[] ansb = answerIF.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_ANSWER, result, ansb);
    }

    public void deflectCall(int index, String number, Message result) {
        logv("deflect call to: " + number + "connid:" + index);
        ImsQmiIF.DeflectCall deflectCall = new ImsQmiIF.DeflectCall();
        deflectCall.setConnIndex(index);
        deflectCall.setNumber(number);
        byte[] deflectCallb = deflectCall.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_DEFLECT_CALL, result, deflectCallb);
    }

    /* Not used yet - TODO figure out how to get this presentation value before calling */
    public void
    acceptCall(Message result, int callType, int presentation) {
        logv("acceptCall callType= " + callType + " presentation= " + presentation);
        int callTypeIF = callType;
        ImsQmiIF.Answer answerIF = new ImsQmiIF.Answer();
        answerIF.setCallType(callTypeIF);
        answerIF.setPresentation(presentation);
        byte[] ansb = answerIF.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_ANSWER, result, ansb);
    }

    public void
    hangupConnection(int index, Message result) {
        logv("hangupConnection index= " + index);
        ImsQmiIF.Hangup hangupIF = new ImsQmiIF.Hangup();
        hangupIF.setConnIndex(index);
        hangupIF.setMultiParty(false);
        byte[] hangupb = hangupIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_HANGUP, result, hangupb);
    }

    public void
    hangupWithReason(int connectionId, String userUri, String confUri,
            boolean mpty, int failCause, String errorInfo, Message result) {
        logv("hangupWithReason connId= " + connectionId + " userUri= "+ userUri + " confUri= "+
                confUri + "mpty = " + mpty + "failCause = " + failCause);
        ImsQmiIF.Hangup hangupIF = new ImsQmiIF.Hangup();
        /* If Calltracker has a matching local connection the connection id will be used.
         * if there is no matching connection object and if it is a remotely added participant
         * then connection id will not be present hence -1
         */
        if(connectionId != -1) {
            hangupIF.setConnIndex(connectionId);
        }
        hangupIF.setMultiParty(mpty);
        if(userUri != null)
            hangupIF.setConnUri(userUri);
        ImsQmiIF.CallFailCauseResponse callfail = new ImsQmiIF.CallFailCauseResponse();
        if (errorInfo != null && !errorInfo.isEmpty()) {
            logv("hangupWithReason errorInfo = " + errorInfo);
            ByteStringMicro errorInfoStringMicro = ByteStringMicro.copyFrom(errorInfo.getBytes());
            callfail.setErrorinfo(errorInfoStringMicro);
        }

        int callFailCause = getCallFailCauseForImsReason(failCause);
        callfail.setFailcause(callFailCause);
        logv("hangupWithReason callFailCause=" + callFailCause);
        // Check for unsupported call end reason. If so, set
        // the errorInfo string to the reason code, similar to KK.
        if (callFailCause == ImsQmiIF.CALL_FAIL_MISC) {
            ByteStringMicro errorInfoStringMicro = ByteStringMicro.copyFrom((Integer
                    .toString(failCause)).getBytes());
            callfail.setErrorinfo(errorInfoStringMicro);
            logv("hangupWithReason MISC callFailCause, errorInfo=" + failCause);
        }
        hangupIF.setFailCauseResponse(callfail);

        /* TODO: Change proto file for conf_id to confUri and then enable the line below
        This okay for now as there is not more than one conference simultaneously */
        //hangupIF.setConfUri(confUri);
        byte[] hangupb = hangupIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_HANGUP, result, hangupb);
    }

    public void
    getLastCallFailCause(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_LAST_CALL_FAIL_CAUSE, result, null);
    }

    public void queryServiceStatus(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS, result, null);
    }

    public void setServiceStatus(Message result, int srvType, int network, int enabled,
            int restrictCause) {
        ImsQmiIF.StatusForAccessTech srvSetting = new ImsQmiIF.StatusForAccessTech();
        srvSetting.setNetworkMode(network);
        srvSetting.setStatus(enabled); /*
                                        * TODO: Switch when values of enabled
                                        * and restrictCause are defined in
                                        * Phone.java
                                        */
        srvSetting.setRestrictionCause(restrictCause);
        ImsQmiIF.Info srvInfo = new ImsQmiIF.Info();
        srvInfo.setIsValid(true);
        srvInfo.setCallType(srvType);
        srvInfo.addAccTechStatus(srvSetting);
        encodeMsg(ImsQmiIF.REQUEST_SET_SERVICE_STATUS, result, srvInfo.toByteArray());
    }

    public void
    getCurrentCalls(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_GET_CURRENT_CALLS, result, null);
    }

    public void
            getImsRegistrationState(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE, result, null);
    }

    public void
            sendImsRegistrationState(int imsRegState, Message result) {
        logv("sendImsRegistration " + "imsRegState= " + imsRegState);

        ImsQmiIF.Registration registerImsIF = new ImsQmiIF.Registration();
        registerImsIF.setState(imsRegState);

        byte[] registerImsb = registerImsIF.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_IMS_REG_STATE_CHANGE, result, registerImsb);

   }

    private byte[] setCallModify(CallModify callModify) {
        logv("setCallModify callModify= " + callModify);
        ImsQmiIF.CallDetails callDetailsIF = new ImsQmiIF.CallDetails();
        callDetailsIF.setCallType(callModify.call_details.call_type);
        callDetailsIF.setCallDomain(callModify.call_details.call_domain);

        ImsQmiIF.CallModify callModifyIF = new ImsQmiIF.CallModify();
        callModifyIF.setCallDetails(callDetailsIF);
        callModifyIF.setCallIndex(callModify.call_index);

        // This field is not used for outgoing requests.
        // callModifyIF.setError(callModify.error);

        byte[] callModifyb = callModifyIF.toByteArray();
        return callModifyb;
    }

    public void modifyCallInitiate(Message result, CallModify callModify) {
        logv("modifyCallInitiate callModify= " + callModify);
        byte[] callModifyb = setCallModify(callModify);
        encodeMsg(ImsQmiIF.REQUEST_MODIFY_CALL_INITIATE, result, callModifyb);
    }

    public void modifyCallConfirm(Message result, CallModify callModify) {
        logv("modifyCallConfirm callModify= " + callModify);
        byte[] callModifyb = setCallModify(callModify);
        encodeMsg(ImsQmiIF.REQUEST_MODIFY_CALL_CONFIRM, result, callModifyb);
    }

    public void switchWaitingOrHoldingAndActive(Message result, int callType) {
        logv("switchWaitingOrHoldingAndActive callType=" + callType);
        ImsQmiIF.SwitchWaitingOrHoldingAndActive switchIF
            = new ImsQmiIF.SwitchWaitingOrHoldingAndActive();
        switchIF.setCallType(callType);
        byte[] switchb = switchIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, result, switchb);
    }

    public void switchWaitingOrHoldingAndActive(Message result) {
        ImsQmiIF.SwitchWaitingOrHoldingAndActive switchIF
            = new ImsQmiIF.SwitchWaitingOrHoldingAndActive();
        byte[] switchb = switchIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, result, switchb);
    }

    public void hold(Message result, int callId) {
        ImsQmiIF.Hold holdIF = new ImsQmiIF.Hold();
        holdIF.setCallId(callId);
        byte[] holdb = holdIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_HOLD, result, holdb);
    }

    public void resume(Message result, int callId) {
        ImsQmiIF.Resume resumeIF = new ImsQmiIF.Resume();
        resumeIF.setCallId(callId);
        byte[] resumeb = resumeIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_RESUME, result, resumeb);
    }

    public void hangupForegroundResumeBackground(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, result, null);
    }

    public void hangupWaitingOrBackground(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_HANGUP_WAITING_OR_BACKGROUND, result, null);
    }

    public void conference(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_CONFERENCE, result, null);
    }

    public void explicitCallTransfer(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_EXPLICIT_CALL_TRANSFER, result, null);
    }

    public void rejectCall(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_UDUB, result, null);
    }

    public void sendDtmf(char c, Message result) {
        ImsQmiIF.Dtmf dtmfIF = new ImsQmiIF.Dtmf();
        dtmfIF.setDtmf(Character.toString(c));
        byte[] dtmfb = dtmfIF.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_DTMF, result, dtmfb);
    }

    public void startDtmf(char c, Message result) {
        ImsQmiIF.Dtmf dtmfIF = new ImsQmiIF.Dtmf();
        dtmfIF.setDtmf(Character.toString(c));

        byte[] dtmfb = dtmfIF.toByteArray();

        encodeMsg(ImsQmiIF.REQUEST_DTMF_START, result, dtmfb);
    }

    public void stopDtmf(Message result) {
        encodeMsg(ImsQmiIF.REQUEST_DTMF_STOP, result, null);
    }

    // RESPONSE PROCESSING
    private Object responseCallFailCause(byte[] callFailB) {
        log(" responseCallFailCause ");
        ImsQmiIF.CallFailCauseResponse callfail = null;
        try {

            callfail = ImsQmiIF.CallFailCauseResponse
                    .parseFrom(callFailB);
            log("callfail cause response" + callfail);
        } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
            log(" Message parsing ");
            log("InvalidProtocolBufferException ");
        }
        return callfail;

    }

    private Object responseCallRingBack(byte[] ringBackB) {
        int[] response = new int[1];
        log(" responseCallRingBack ");

        try {
            ImsQmiIF.RingBackTone ringbackTone = ImsQmiIF.RingBackTone
                    .parseFrom(ringBackB);

            response[0] = ringbackTone.getFlag();
            log("responseCallRingBack " + response[0]);
        } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
            log(" Message parsing ");
            log("InvalidProtocolBufferException ");
        }
        return response;
    }

    protected Object responseImsRegistration(byte[] imsRegB) {
        log(" responseImsRegistration");
        ImsQmiIF.Registration registration = null;
        if (imsRegB != null && imsRegB.length >= 1) {
            try {
                registration = ImsQmiIF.Registration.parseFrom(imsRegB);
                log("Ims registration" + registration);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        } else {
            Log.e(LOG_TAG, "responseImsRegistration error");
        }
        return registration;
    }

    protected Object responseQueryCallForward(byte[] callInfoList) {

        CallForwardInfo infos[] = null;
        int numInfos = 0;

        if (callInfoList != null) {
            try {
                ImsQmiIF.CallForwardInfoList infoList = ImsQmiIF.CallForwardInfoList
                        .parseFrom(callInfoList);
                numInfos = infoList.getInfoCount();
                infos = new CallForwardInfo[numInfos];
                for (int i = 0; i < numInfos; i++) {
                    ImsQmiIF.CallForwardInfoList.CallForwardInfo callInfo = infoList
                            .getInfo(i);

                    infos[i] = new CallForwardInfo();
                    infos[i].status = callInfo.getStatus();
                    infos[i].reason = callInfo.getReason();
                    infos[i].serviceClass = callInfo.getServiceClass();
                    infos[i].toa = callInfo.getToa();
                    infos[i].number = callInfo.getNumber();
                    infos[i].timeSeconds = callInfo.getTimeSeconds();
                    if (callInfo.hasCallFwdTimerStart() &&
                            callInfo.hasCallFwdTimerEnd()) {
                        ImsQmiIF.CallFwdTimerInfo startCallTimerInfo =
                                callInfo.getCallFwdTimerStart();
                        infos[i].startHour = startCallTimerInfo.getHour();
                        infos[i].startMinute = startCallTimerInfo.getMinute();
                        ImsQmiIF.CallFwdTimerInfo endCallTimerInfo =
                                callInfo.getCallFwdTimerEnd();
                        infos[i].endHour = endCallTimerInfo.getHour();
                        infos[i].endMinute = endCallTimerInfo.getMinute();
                    }
                }

            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        } else {
            infos = new CallForwardInfo[0];
        }
        return infos;
    }

    protected Object responseQueryCallWaiting(byte[] callWaitingInfo) {
        int[] response = null;

        if (callWaitingInfo != null) {
            try {
                ImsQmiIF.CallWaitingInfo waitingInfo = ImsQmiIF.CallWaitingInfo
                        .parseFrom(callWaitingInfo);
                ImsQmiIF.ServiceClass srvClass = waitingInfo.getServiceClass();

                if (waitingInfo.getServiceStatus() == ImsQmiIF.DISABLED) {
                    response = new int[1];
                    response[0] = ImsQmiIF.DISABLED;
                } else {
                    response = new int[2];
                    response[0] = ImsQmiIF.ENABLED;
                    response[1] = srvClass.getServiceClass();
                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        } else {
            response = new int[0];
        }
        return response;
    }

    protected Object responseQueryClip(byte[] clipInfo) {
        int[] response = null;

        if (clipInfo != null) {
            try {
                ImsQmiIF.ClipProvisionStatus clipStatus = ImsQmiIF.ClipProvisionStatus
                        .parseFrom(clipInfo);
                response = new int[1];
                response[0] = clipStatus.getClipStatus();
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        }
        else {
            response = new int[0];
        }
        return response;
    }

    protected Object responseGetClir(byte[] clirInfo) {
        int[] response = null;

        if (clirInfo != null) {
            try {
                ImsQmiIF.Clir info = ImsQmiIF.Clir.parseFrom(clirInfo);

                response = new int[2];

                response[0] = info.getParamN();
                response[1] = info.getParamM();
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        }
        else
        {
            response = new int[0];
        }
        return response;
    }

    protected Object responseGetColr(byte[] colrInfo) {
        int[] response = null;

        if (colrInfo != null) {
            try {
                ImsQmiIF.Colr info = ImsQmiIF.Colr.parseFrom(colrInfo);
                response = new int[1];
                response[0] = info.getPresentation();
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        } else {
            response = new int[0];
        }
        return response;
    }

    protected Object responseHandover(byte[] handoverB) {
        ImsQmiIF.Handover handover = null;
        if (handoverB != null && handoverB.length >= 1) {
            try {
                handover = ImsQmiIF.Handover.parseFrom(handoverB);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in responseHandover parsing");
            }
        }
        return handover;
    }

    protected Object handleRadioStateChange(byte[] radioStateChange) {
        int[] response = null;
        if (radioStateChange != null && radioStateChange.length >= 1) {
            try {
                ImsQmiIF.RadioStateChanged state = ImsQmiIF.RadioStateChanged
                        .parseFrom(radioStateChange);
                response = new int[1];
                response[0] = state.getState();
                switch (state.getState()) {
                    case ImsQmiIF.RADIO_STATE_OFF:
                        setRadioState(RadioState.RADIO_OFF);
                        break;
                    case ImsQmiIF.RADIO_STATE_UNAVAILABLE:
                        setRadioState(RadioState.RADIO_UNAVAILABLE);
                        break;
                    case ImsQmiIF.RADIO_STATE_ON:
                        setRadioState(RadioState.RADIO_ON);
                        break;
                    default:
                        Log.e(LOG_TAG, "Invalid state in Radio State Change");
                        break;

                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in handleRadioStateChange parsing");
            }
        }
        return response;
    }

    protected Object handleMwi(byte[] mwiNotification) {
        ImsQmiIF.Mwi notification = null;
        if (mwiNotification != null && mwiNotification.length >= 1) {
            try {
                notification = ImsQmiIF.Mwi.parseFrom(mwiNotification);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in handleMwi parsing");
            }
        }
        return notification;
    }

    protected Object
    responseSuppServiceNotification(byte[] suppSrvNotification) {
        ImsQmiIF.SuppSvcNotification notification = null;
        if (suppSrvNotification != null && suppSrvNotification.length >= 1 ) {
            try {
                notification =
                        ImsQmiIF.SuppSvcNotification.parseFrom(suppSrvNotification);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in responseSuppServiceNotification parsing");
            }
        }
        return notification;
    }

    protected Object handleRefreshInfo(byte[] confInfo) {
        ImsQmiIF.ConfInfo info = null;
        if (confInfo != null && confInfo.length >= 1) {
            try {
                info = ImsQmiIF.ConfInfo.parseFrom(confInfo);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in responseHandover parsing");
            }
        }
        return info;
    }

    private void disableSrvStatus() {
        Log.v(LOG_TAG, "disableSrvStatus");
        if (mSrvStatusRegistrations != null) {
            mSrvStatusRegistrations
                    .notifyRegistrants(new AsyncResult(null, null, new IOException()));
        }
    }

    private void unpackAccTechStatus(ImsQmiIF.Info info, ServiceStatus srvSt) {
        int numAccessTechUpdate = 0;

        numAccessTechUpdate = info.getAccTechStatusCount();

        srvSt.accessTechStatus = new ServiceStatus.
                StatusForAccessTech[numAccessTechUpdate];
        for (int j = 0; j < numAccessTechUpdate; j++) {
            ImsQmiIF.StatusForAccessTech update = info.getAccTechStatus(j);
            srvSt.accessTechStatus[j] = new ServiceStatus.
                    StatusForAccessTech();
            srvSt.accessTechStatus[j].networkMode = update.getNetworkMode();
            srvSt.accessTechStatus[j].status = update.getStatus();
            srvSt.accessTechStatus[j].restrictCause = update.getRestrictionCause();
            if (update.getRegistered() != null) { // Registered is
                                                  // optional field
                srvSt.accessTechStatus[j].registered = update.getRegistered()
                        .getState();
            } else {
                srvSt.accessTechStatus[j].registered = ImsQmiIF.Registration.
                        NOT_REGISTERED;
                            Log.e(LOG_TAG, "Registered not sent");
            }
            log(" networkMode = " + srvSt.accessTechStatus[j].networkMode +
                    " status = " + srvSt.accessTechStatus[j].status +
                    " restrictCause = " + srvSt.accessTechStatus[j].restrictCause +
                    " registered = " + srvSt.accessTechStatus[j].registered);
       }
    }

    protected Object handleSrvStatus(byte[] updateList) {
        ArrayList<ServiceStatus> response = null;
        int num = 0;

        if (updateList != null) {
            try {
                ImsQmiIF.SrvStatusList statusList = ImsQmiIF.SrvStatusList.parseFrom(updateList);
                num = statusList.getSrvStatusInfoCount();
                response = new ArrayList<ServiceStatus>(num);
                ServiceStatus srvSt;

                for (int i = 0; i < num; i++) {
                    ImsQmiIF.Info info = statusList.getSrvStatusInfo(i);
                    srvSt = new ServiceStatus();
                    srvSt.isValid = info.getIsValid();
                    srvSt.type = info.getCallType();
                    srvSt.status = info.getStatus();
                    if (info.getUserdata().size() > 0) {
                        srvSt.userdata = new byte[info.getUserdata().size()];
                        info.getUserdata().copyTo(srvSt.userdata, 0);
                    }
                    log("isValid = " + srvSt.isValid + " type = " + srvSt.type + " status = " +
                            srvSt.status + " userdata = " + srvSt.userdata);

                    unpackAccTechStatus(info, srvSt);
                    response.add(srvSt);
                }
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in handleSrvStatus parsing");
            }
        }
        else {
            response = new ArrayList<ServiceStatus>(num);
        }
        return response;
    }

    protected Object handleTtyNotify(byte[] notification) {
        int[] mode = null;
        if (notification != null) {
            try {
                ImsQmiIF.TtyNotify notify = ImsQmiIF.TtyNotify.parseFrom(notification);
                mode = new int[1];
                mode[0] = notify.getMode();
                Log.d(LOG_TAG, "handleTtyNotify mode = " + mode[0]);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in Message Tag parsing ");
            }
        }
        return mode;
    }

    private ServiceStatus[] copySrvStatusList(ServiceStatus[] toList,
            ImsQmiIF.SrvStatusList fromList) {
        if (fromList != null) {
            toList = new ServiceStatus[fromList.getSrvStatusInfoCount()];
            Log.v(LOG_TAG, "Num of SrvUpdates = " + fromList.getSrvStatusInfoCount());
            for (int i = 0; i < fromList.getSrvStatusInfoCount(); i++) {
                ImsQmiIF.Info info = fromList.getSrvStatusInfo(i);
                if (info != null && toList != null) {
                    toList[i] = new ServiceStatus();
                    toList[i].isValid = info.getIsValid();
                    toList[i].type = info.getCallType();
                    if (info.getAccTechStatusCount() >= 1) {
                        unpackAccTechStatus(info, toList[i]);
                    } else {
                        toList[i].accessTechStatus = new ServiceStatus.StatusForAccessTech[1];
                        toList[i].accessTechStatus[0] = new ServiceStatus.StatusForAccessTech();
                        ServiceStatus.StatusForAccessTech act = toList[i].accessTechStatus[0];
                        act.networkMode = ImsQmiIF.RADIO_TECH_LTE;
                        act.status = info.getStatus();
                        act.restrictCause = info.getRestrictCause();
                    }
                    if (info.getStatus() == ImsQmiIF.STATUS_ENABLED &&
                            info.getRestrictCause() != CallDetails.CALL_RESTRICT_CAUSE_NONE) {
                        Log.v(LOG_TAG, "Partially Enabled Status due to Restrict Cause");
                        toList[i].status = ImsQmiIF.STATUS_PARTIALLY_ENABLED;
                    } else {
                        toList[i].status = info.getStatus();
                    }
                    if (info.getUserdata().size() > 0) {
                        toList[i].userdata = new byte[info.getUserdata().size()];
                        info.getUserdata().copyTo(toList[i].userdata, 0);
                    }
                } else {
                    Log.e(LOG_TAG, "Null service status in list");
                }
            }
        }
        return toList;
    }

    protected Object responseCallList(byte[] callListB) {
        ArrayList<DriverCallIms> response = null;
        int num = 0;

        if (callListB != null) {
            try {
                ImsQmiIF.CallList callList = ImsQmiIF.CallList.parseFrom(callListB);
                num = callList.getCallAttributesCount();

                int voiceSettings;
                DriverCallIms dc;

                response = new ArrayList<DriverCallIms>(num);

                for (int i = 0; i < num; i++) {
                    ImsQmiIF.CallList.Call call = callList.getCallAttributes(i);
                    dc = new DriverCallIms();

                    dc.state = DriverCallIms.stateFromCall(call.getState());
                    dc.index = call.getIndex();
                    dc.TOA = call.getToa();
                    dc.isMpty = call.getIsMpty();
                    dc.isMT = call.getIsMT();
                    dc.als = call.getAls();
                    dc.isVoice = call.getIsVoice();
                    dc.isVoicePrivacy = call.getIsVoicePrivacy();
                    dc.number = call.getNumber();
                    int np = call.getNumberPresentation();
                    dc.numberPresentation = DriverCallIms.presentationFromCLIP(np);
                    dc.name = call.getName();
                    dc.namePresentation = DriverCallIms.presentationFromCLIP(
                            call.getNamePresentation());

                    dc.callDetails = new CallDetails();
                    dc.callDetails.call_type = call.getCallDetails().getCallType();
                    dc.callDetails.call_domain = call.getCallDetails().getCallDomain();
                    dc.callDetails.callsubstate = call.getCallDetails().getCallSubstate();
                    dc.callDetails.callMediaId = call.getCallDetails().getMediaId();
                    List<String> extrasList = call.getCallDetails().getExtrasList();
                    dc.callDetails.extras = extrasList.toArray(new String[extrasList.size()]);
                    dc.callDetails.localAbility = copySrvStatusList(dc.callDetails.localAbility,
                            call.getCallDetails().getLocalAbility());
                    dc.callDetails.peerAbility = copySrvStatusList(dc.callDetails.peerAbility,
                            call.getCallDetails().getPeerAbility());
                    Log.v(LOG_TAG, "Call Details = " + dc.callDetails);
                    // Make sure there's a leading + on addresses with a TOA of
                    // 145
                    dc.number = PhoneNumberUtils.stringFromStringAndTOA(dc.number, dc.TOA);

                    // Create a default ImsReasonInfo to avoid null-checks
                    // in other places.
                    dc.callFailCause =
                            new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED, 0);

                    if (call.hasFailCause()) {
                        String networkError = null;
                        // Check for an error message from the network.
                        if (call.getFailCause().hasNetworkErrorString()) {
                            networkError = call.getFailCause().getNetworkErrorString();
                            // If network sends a "Forbidden - Not authorized for service" string,
                            // throw an intent. This intent is expected to be processed by OMA-DM
                            // applications.
                            if (networkError.equals(
                                    ImsReasonInfo.EXTRA_MSG_SERVICE_NOT_AUTHORIZED)) {
                                log("Throwing ACTION_FORBIDDEN_NO_SERVICE_AUTHORIZATION intent.");
                                Intent intent = new Intent(
                                        TelephonyIntents.ACTION_FORBIDDEN_NO_SERVICE_AUTHORIZATION);
                                intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                                mContext.sendBroadcast(intent);
                            }
                        }
                        // Check if the CallFailCauseResponse has an error code.
                        if (call.getFailCause().hasFailcause()) {
                            dc.callFailCause.mCode
                                = getImsReasonForCallFailCause(call.getFailCause());
                            // If there is a network error, propagate it through
                            // the ImsReasonInfo object.
                            if (networkError != null) {
                                dc.callFailCause.mExtraMessage = networkError;
                            } else if (call.getFailCause().hasErrorinfo() &&
                                    call.getFailCause().getErrorinfo() != null) {
                                // Pass the optional error info to upper layers.
                                // To maintain backward compatibility for apps like CSVT.
                                dc.callFailCause.mExtraMessage
                                        = call.getFailCause().getErrorinfo().toStringUtf8();
                            }
                        } else {
                            log("CallFailCauseResponse has no int error code!");
                        }
                    }

                    response.add(dc);

                    if (dc.isVoicePrivacy) {
                        mVoicePrivacyOnRegistrants.notifyRegistrants();
                        log("InCall VoicePrivacy is enabled");
                    } else {
                        mVoicePrivacyOffRegistrants.notifyRegistrants();
                        log("InCall VoicePrivacy is disabled");
                    }
                }

                Collections.sort(response);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in Message Tag parsing ");
            }
        }
        else
            response = new ArrayList<DriverCallIms>(num); //empty array

        return response;
    }

    private int getImsReasonForCallFailCause(ImsQmiIF.CallFailCauseResponse failCauseResp) {
        int failCause = failCauseResp.getFailcause();
        int imsCode = ImsReasonInfo.CODE_UNSPECIFIED;
        log("Call fail cause= " + failCause);

        switch (failCause) {

            // SIP Codes
            case ImsQmiIF.CALL_FAIL_SIP_REDIRECTED:
                imsCode = ImsReasonInfo.CODE_SIP_REDIRECTED;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_BAD_REQUEST:
                imsCode = ImsReasonInfo.CODE_SIP_BAD_REQUEST;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_FORBIDDEN:
                imsCode = ImsReasonInfo.CODE_SIP_FORBIDDEN;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_NOT_FOUND:
                imsCode = ImsReasonInfo.CODE_SIP_NOT_FOUND;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_NOT_SUPPORTED:
                imsCode = ImsReasonInfo.CODE_SIP_NOT_SUPPORTED;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_REQUEST_TIMEOUT:
                imsCode = ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE:
                imsCode = ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_BAD_ADDRESS:
                imsCode = ImsReasonInfo.CODE_SIP_BAD_ADDRESS;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_BUSY:
                imsCode = ImsReasonInfo.CODE_SIP_BUSY;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_REQUEST_CANCELLED:
                imsCode = ImsReasonInfo.CODE_SIP_REQUEST_CANCELLED;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_NOT_ACCEPTABLE:
                imsCode = ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_NOT_REACHABLE:
                imsCode = ImsReasonInfo.CODE_SIP_NOT_REACHABLE;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_SERVER_INTERNAL_ERROR:
                imsCode = ImsReasonInfo.CODE_SIP_SERVER_INTERNAL_ERROR;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_SERVICE_UNAVAILABLE:
                imsCode = ImsReasonInfo.CODE_SIP_SERVICE_UNAVAILABLE;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_SERVER_TIMEOUT:
                imsCode = ImsReasonInfo.CODE_SIP_SERVER_TIMEOUT;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_USER_REJECTED:
                imsCode = ImsReasonInfo.CODE_SIP_USER_REJECTED;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_GLOBAL_ERROR:
                imsCode = ImsReasonInfo.CODE_SIP_GLOBAL_ERROR;
                break;
            // Media Codes
            case ImsQmiIF.CALL_FAIL_MEDIA_INIT_FAILED:
                imsCode = ImsReasonInfo.CODE_MEDIA_INIT_FAILED;
                break;
            case ImsQmiIF.CALL_FAIL_MEDIA_NO_DATA:
                imsCode = ImsReasonInfo.CODE_MEDIA_NO_DATA;
                break;
            case ImsQmiIF.CALL_FAIL_MEDIA_NOT_ACCEPTABLE:
                imsCode = ImsReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE;
                break;
            case ImsQmiIF.CALL_FAIL_MEDIA_UNSPECIFIED_ERROR:
                imsCode = ImsReasonInfo.CODE_MEDIA_UNSPECIFIED;
                break;

            case ImsQmiIF.CALL_FAIL_NORMAL:
                imsCode = ImsReasonInfo.CODE_USER_TERMINATED;
                break;
            case ImsQmiIF.CALL_FAIL_BUSY:
                imsCode = ImsReasonInfo.CODE_SIP_BUSY;
                break;
            case ImsQmiIF.CALL_FAIL_NETWORK_UNAVAILABLE:
                imsCode = ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE;
                break;
            case ImsQmiIF.CALL_FAIL_ANSWERED_ELSEWHERE:
                imsCode = ImsReasonInfo.CODE_ANSWERED_ELSEWHERE;
                break;
            case ImsQmiIF.CALL_FAIL_EMERGENCY_TEMP_FAILURE:
                imsCode = ImsReasonInfo.CODE_EMERGENCY_TEMP_FAILURE;
                break;
            case ImsQmiIF.CALL_FAIL_EMERGENCY_PERM_FAILURE:
                imsCode = ImsReasonInfo.CODE_EMERGENCY_PERM_FAILURE;
                break;
            case ImsQmiIF.CALL_FAIL_FDN_BLOCKED:
                imsCode = ImsReasonInfo.CODE_FDN_BLOCKED;
                break;
            case ImsQmiIF.CALL_FAIL_SIP_Error:
            case ImsQmiIF.CALL_FAIL_UNOBTAINABLE_NUMBER:
            case ImsQmiIF.CALL_FAIL_CONGESTION:
            case ImsQmiIF.CALL_FAIL_INCOMPATIBILITY_DESTINATION:
            case ImsQmiIF.CALL_FAIL_CALL_BARRED:
            case ImsQmiIF.CALL_FAIL_FEATURE_UNAVAILABLE:
            case ImsQmiIF.CALL_FAIL_ERROR_UNSPECIFIED:
            default:
                imsCode = ImsReasonInfo.CODE_UNSPECIFIED;
        }

        return imsCode;
    }

    private int getCallFailCauseForImsReason(int imsReason) {
        log("imsReason= " + imsReason);
        int failCause;

        switch (imsReason) {
            case ImsReasonInfo.CODE_USER_DECLINE:
                failCause = ImsQmiIF.CALL_FAIL_USER_REJECT;
                break;
            case ImsReasonInfo.CODE_USER_TERMINATED:
                failCause = ImsQmiIF.CALL_FAIL_USER_BUSY;
                break;
            case ImsReasonInfo.CODE_LOW_BATTERY:
                failCause = ImsQmiIF.CALL_FAIL_LOW_BATTERY;
                break;
            case ImsReasonInfo.CODE_BLACKLISTED_CALL_ID:
                failCause = ImsQmiIF.CALL_FAIL_BLACKLISTED_CALL_ID;
                break;

            default:
                log("Unsupported imsReason for ending call. Passing end cause as 'misc'.");
                return ImsQmiIF.CALL_FAIL_MISC;
        }
        return failCause;
    }

    protected Object responseSuppSvcStatus(byte[] suppSvcStatusInfo) {
        ImsQmiIF.SuppSvcResponse response = null;

        if (suppSvcStatusInfo != null) {
            try {
                response = ImsQmiIF.SuppSvcResponse.parseFrom(suppSvcStatusInfo);
            } catch (com.google.protobuf.micro.InvalidProtocolBufferMicroException ex) {
                log("InvalidProtocolBufferException in message tag parsing");
            }
        } else {
            log("responseSuppSvcStatus suppSvcStatusInfo null");
        }
        return response;
    }

    public void setSuppServiceNotifications(boolean enable, Message result) {
        logv("setSuppServiceNotifications enable = " + enable);
        ImsQmiIF.SuppSvcStatus svcStatus = new ImsQmiIF.SuppSvcStatus();
        svcStatus.setStatus(enable ? ImsQmiIF.ENABLED
                : ImsQmiIF.DISABLED);
        byte[] suppServiceNotif = svcStatus.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION, result, suppServiceNotif);
    }

    public void getCLIR(Message result) {
        logv("getCLIR");
        encodeMsg(ImsQmiIF.REQUEST_GET_CLIR, result, null);
    }

    public void setCLIR(int clirMode, Message result) {
        logv("setCLIR clirmode = " + clirMode);
        ImsQmiIF.Clir clirValue = new ImsQmiIF.Clir();
        // clirValue.param_n = clirMode;
        clirValue.setParamN(clirMode);
        byte[] setCLIRInfo = clirValue.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_CLIR, result, setCLIRInfo);
    }

    public void queryCallWaiting(int serviceClass, Message response) {
        logv("queryCallWaiting serviceClass = " + serviceClass);
        ImsQmiIF.ServiceClass callWaitingQuery = new ImsQmiIF.ServiceClass();
        callWaitingQuery.setServiceClass(serviceClass);
        byte[] callWaitingQueryInfo = callWaitingQuery.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_QUERY_CALL_WAITING, response,
                callWaitingQueryInfo);
    }

    public void setCallWaiting(boolean enable, int serviceClass,
            Message response) {
        logv("setCallWaiting enable = " + enable + "serviceClass = "
                + serviceClass);
        ImsQmiIF.CallWaitingInfo setCallWaiting = new ImsQmiIF.CallWaitingInfo();
        ImsQmiIF.ServiceClass sc = new ImsQmiIF.ServiceClass();
        sc.setServiceClass(serviceClass);
        setCallWaiting.setServiceStatus(enable ? ImsQmiIF.ENABLED
                : ImsQmiIF.DISABLED);
        setCallWaiting.setServiceClass(sc);
        byte[] callWaitingSetInfo = setCallWaiting.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_CALL_WAITING, response,
                callWaitingSetInfo);
    }

    public void queryIncomingCallBarring(String facility, int serviceClass, Message response) {
        suppSvcStatus(ImsQmiIF.QUERY, facilityStringToInt(facility), null, serviceClass, response);
    }

    public void setIncomingCallBarring(int operation, String facility, String[] icbNum,
            int serviceClass, Message response) {
        suppSvcStatus(operation, facilityStringToInt(facility), icbNum, serviceClass, response);
    }

    public void setCallForward(int action, int cfReason, int serviceClass,
            String number, int timeSeconds, Message response) {
        logv("setCallForward cfReason= " + cfReason + " serviceClass = "
                + serviceClass + "number = " + number + "timeSeconds = "
                + timeSeconds);
        ImsQmiIF.CallForwardInfoList callForwardIF = new ImsQmiIF.CallForwardInfoList();
        ImsQmiIF.CallForwardInfoList.CallForwardInfo callInfo = new ImsQmiIF.CallForwardInfoList.CallForwardInfo();
        callInfo.setStatus(action);
        callInfo.setReason(cfReason);
        callInfo.setServiceClass(serviceClass);
        callInfo.setToa(PhoneNumberUtils.toaFromString(number));
        if (number != null)
            callInfo.setNumber(number);
        callInfo.setTimeSeconds(timeSeconds);
        callForwardIF.addInfo(callInfo);
        byte[] setCallForwardInfo = callForwardIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS, response,
                setCallForwardInfo);
    }

    public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour,
            int endMinute, int action, int cfReason, int serviceClass,
            String number, Message response) {
        logv("setCallForwardUncondTimer cfReason= " + cfReason + " serviceClass = "
                + serviceClass + "number = " + number + "startHour = " + startHour
                + "startMinute = " + startMinute + "endHour = " + endHour + "endMin = " + endMinute);
        ImsQmiIF.CallForwardInfoList callForwardIF = new ImsQmiIF.CallForwardInfoList();
        ImsQmiIF.CallForwardInfoList.CallForwardInfo callInfo =
                new ImsQmiIF.CallForwardInfoList.CallForwardInfo();
        callInfo.setStatus(action);
        callInfo.setReason(cfReason);
        callInfo.setServiceClass(serviceClass);
        callInfo.setToa(PhoneNumberUtils.toaFromString(number));
        if (number != null)
            callInfo.setNumber(number);
        callInfo.setTimeSeconds(0);

        ImsQmiIF.CallFwdTimerInfo startCallTimerInfo = new ImsQmiIF.CallFwdTimerInfo();
        startCallTimerInfo.setHour(startHour);
        startCallTimerInfo.setMinute(startMinute);
        callInfo.setCallFwdTimerStart(startCallTimerInfo);

        ImsQmiIF.CallFwdTimerInfo endCallTimerInfo = new ImsQmiIF.CallFwdTimerInfo();
        endCallTimerInfo.setHour(endHour);
        endCallTimerInfo.setMinute(endMinute);
        callInfo.setCallFwdTimerEnd(endCallTimerInfo);

        callForwardIF.addInfo(callInfo);
        byte[] setCallForwardInfo = callForwardIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS, response,
                setCallForwardInfo);
    }

    public void queryCallForwardStatus(int cfReason, int serviceClass,
            String number, Message response) {
        logv("queryCallForwardStatus cfReason= " + cfReason
                + " serviceClass = " + serviceClass + "number = " + number);
        ImsQmiIF.CallForwardInfoList callForwardIF = new ImsQmiIF.CallForwardInfoList();
        ImsQmiIF.CallForwardInfoList.CallForwardInfo callInfo = new ImsQmiIF.CallForwardInfoList.CallForwardInfo();
        callInfo.setStatus(2);
        callInfo.setReason(cfReason);
        callInfo.setServiceClass(serviceClass);
        callInfo.setToa(PhoneNumberUtils.toaFromString(number));
        if (number != null)
            callInfo.setNumber(number);
        callInfo.setTimeSeconds(0);
        callForwardIF.addInfo(callInfo);
        byte[] callForwardQuery = callForwardIF.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_QUERY_CALL_FORWARD_STATUS, response,
                callForwardQuery);
    }

    public void queryCLIP(Message response) {
        logv("queryClip");
        encodeMsg(ImsQmiIF.REQUEST_QUERY_CLIP, response, null);
    }

    public void setUiTTYMode(int uiTtyMode, Message response) {
        Log.d(LOG_TAG, "setUiTTYMode uittyMode=" + uiTtyMode);

        ImsQmiIF.TtyNotify notify = new ImsQmiIF.TtyNotify();
        notify.setMode(uiTtyMode);
        byte[] setTtyInfo = notify.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SEND_UI_TTY_MODE, response,
                setTtyInfo);
    }

    public void exitEmergencyCallbackMode(Message response) {
        logv("exitEmergencyCallbackMode");
        encodeMsg(ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, response, null);
    }

    @Override
    public void queryFacilityLock(String facility, String password,
            int serviceClass, Message response) {
        suppSvcStatus(ImsQmiIF.QUERY, facilityStringToInt(facility), response);
    }

    @Override
    public void setFacilityLock(String facility, boolean lockState,
            String password, int serviceClass, Message response) {
        int operation = lockState ? ImsQmiIF.ACTIVATE : ImsQmiIF.DEACTIVATE;
        suppSvcStatus(operation, facilityStringToInt(facility), response);
    }

    public void getSuppSvc(String facility, Message response) {
        suppSvcStatus(ImsQmiIF.QUERY, facilityStringToInt(facility), response);
    }

    public void setSuppSvc(String facility, boolean lockState, Message response) {
        int operation = lockState ? ImsQmiIF.ACTIVATE : ImsQmiIF.DEACTIVATE;
        suppSvcStatus(operation, facilityStringToInt(facility), response);
    }

    public void suppSvcStatus(int operationType, int facility, String[] icbNum,
            int serviceClassValue, Message response) {
        logv("suppSvcStatus operationType = " + operationType + " facility = "
                + facility + "serviceClassValue = " + serviceClassValue);

        ImsQmiIF.SuppSvcRequest supsServiceStatus = new ImsQmiIF.SuppSvcRequest();
        supsServiceStatus.setOperationType(operationType);
        supsServiceStatus.setFacilityType(facility);

        ImsQmiIF.ServiceClass serviceClass = new ImsQmiIF.ServiceClass();
        serviceClass.setServiceClass(serviceClassValue); /* holds service class value
                                                          * i.e 1 for Voice class etc
                                                          */

        ImsQmiIF.CbNumListType cbNumListType = new ImsQmiIF.CbNumListType();
        cbNumListType.setServiceClass(serviceClass);

        if (icbNum != null) {
            for (int i = 0; i <  icbNum.length; i++) {
                logv("icbnum: " + icbNum[i] + "at index: " + i);
                ImsQmiIF.CbNumList cbNumList = new ImsQmiIF.CbNumList();
                cbNumList.setNumber(icbNum[i]);
                cbNumListType.addCbNumList(cbNumList);
            }
        }

        supsServiceStatus.setCbNumListType(cbNumListType);

        byte[] supsService = supsServiceStatus.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SUPP_SVC_STATUS, response,
                supsService);
    }

    public void suppSvcStatus(int operationType, int facility, Message response) {
        logv("suppSvcStatus operationType = " + operationType + " facility = "
                + facility);
        ImsQmiIF.SuppSvcRequest supsServiceStatus = new ImsQmiIF.SuppSvcRequest();
        supsServiceStatus.setOperationType(operationType);
        supsServiceStatus.setFacilityType(facility);
        byte[] supsService = supsServiceStatus.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SUPP_SVC_STATUS, response,
                supsService);
    }

    public void getCOLR(Message result) {
        logv("getCOLR");
        encodeMsg(ImsQmiIF.REQUEST_GET_COLR, result, null);
    }

    public void setCOLR(int presentationValue, Message result) {
        logv("setCOLR presentationValue = " + presentationValue);
        ImsQmiIF.Colr colrValue = new ImsQmiIF.Colr();

        colrValue.setPresentation(presentationValue);
        byte[] setCOLRInfo = colrValue.toByteArray();
        encodeMsg(ImsQmiIF.REQUEST_SET_COLR, result, setCOLRInfo);
    }

    static int facilityStringToInt(String sc) {
        if (sc == null) {
            throw new RuntimeException ("invalid supplementary service");
        }

        if (sc.equals("CLIP")) {
            return ImsQmiIF.FACILITY_CLIP;
        }
        else if (sc.equals("COLP")) {
            return ImsQmiIF.FACILITY_COLP;
        }
/*
        if (sc.equals(VoLteMmiCode.SC_BAOC)) {
            return ImsQmiIF.FACILITY_BAOC;
        } else if (sc.equals(VoLteMmiCode.SC_BAOIC)) {
            return ImsQmiIF.FACILITY_BAOIC;
        } else if (sc.equals(VoLteMmiCode.SC_BAOICxH)) {
            return ImsQmiIF.FACILITY_BAOICxH;
        } else if (sc.equals(VoLteMmiCode.SC_BAIC)) {
            return ImsQmiIF.FACILITY_BAIC;
        } else if (sc.equals(VoLteMmiCode.SC_BAICr)) {
            return ImsQmiIF.FACILITY_BAICr;
        } else if (sc.equals(VoLteMmiCode.SC_BA_ALL)) {
            return ImsQmiIF.FACILITY_BA_ALL;
        } else if (sc.equals(VoLteMmiCode.SC_BA_MO)) {
            return ImsQmiIF.FACILITY_BA_MO;
        } else if (sc.equals(VoLteMmiCode.SC_BA_MT)) {
            return ImsQmiIF.FACILITY_BA_MT;
        } else if (sc.equals(VoLteMmiCode.SC_BS_MT)) {
            return ImsQmiIF.FACILITY_BS_MT;
        } else if (sc.equals(VoLteMmiCode.SC_BAICa)) {
            return ImsQmiIF.FACILITY_BAICa;
        } else if (sc.equals(VoLteMmiCode.SC_CLIP)) {
            return ImsQmiIF.FACILITY_CLIP;
        } else if (sc.equals(VoLteMmiCode.SC_COLP)) {
            return ImsQmiIF.FACILITY_COLP;
        } else {
            throw new RuntimeException ("invalid supplementary service");
        } */
        return 0;
    }

    // Query for current video call quality.
    public void queryVideoQuality(Message response) {
        Log.d(LOG_TAG, "queryVideoQuality");
        encodeMsg(ImsQmiIF.REQUEST_QUERY_VT_CALL_QUALITY, response, null);
    }

     // Set for current video call quality.
    public void setVideoQuality(int quality,  Message response) {
        Log.d(LOG_TAG, "setVideoQuality quality=" + quality);
        ImsQmiIF.VideoCallQuality msgQuality = new ImsQmiIF.VideoCallQuality();
        msgQuality.setQuality(quality);
        encodeMsg(ImsQmiIF.REQUEST_SET_VT_CALL_QUALITY, response, msgQuality.toByteArray());
    }

    public void getPacketCount(Message response) {
        Log.d(LOG_TAG, "getPacketCount");
        encodeMsg(ImsQmiIF.REQUEST_GET_RTP_STATISTICS, response, null);
    }

    public void getPacketErrorCount(Message response) {
        Log.d(LOG_TAG, "getPacketErrorCount");
        encodeMsg(ImsQmiIF.REQUEST_GET_RTP_ERROR_STATISTICS, response, null);
    }

    public void getWifiCallingPreference(Message response) {
        Log.d(LOG_TAG, "getWifiCallingPreference");
        encodeMsg(ImsQmiIF.REQUEST_GET_WIFI_CALLING_STATUS, response, null);
    }

    public void setWifiCallingPreference(int wifiCallingStatus, int wifiCallingPreference,
            Message response) {
        Log.d(LOG_TAG, "setWifiCallingPreference wifiCallingStauts : " + wifiCallingStatus +
            " wifiCallingPreference : " + wifiCallingPreference);
        ImsQmiIF.WifiCallingInfo wifiCallingInfo = new ImsQmiIF.WifiCallingInfo();
        wifiCallingInfo.setStatus(wifiCallingStatus);
        wifiCallingInfo.setPreference(wifiCallingPreference);
        encodeMsg(ImsQmiIF.REQUEST_SET_WIFI_CALLING_STATUS, response,
                wifiCallingInfo.toByteArray());
    }
}
