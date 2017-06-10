/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
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

/*******************************************************************************
@file    QcrilMsgTunnelSocket.java
@brief   Socket Impl between QCRIL and OEM specific reqs/responses
 ******************************************************************************/

package com.qualcomm.qcrilmsgtunnel;

import static com.android.internal.telephony.RILConstants.GENERIC_FAILURE;
import static com.android.internal.telephony.RILConstants.RADIO_NOT_AVAILABLE;
import static com.android.internal.telephony.RILConstants.RIL_REQUEST_OEM_HOOK_RAW;
import static com.android.internal.telephony.RILConstants.RIL_UNSOL_OEM_HOOK_RAW;
import static android.Manifest.permission.READ_PHONE_STATE;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;

import android.app.ActivityManagerNative;
import android.content.Context;
import android.content.Intent;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.PowerManager;
import android.os.UserHandle;
import android.os.PowerManager.WakeLock;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;

public class QcrilMsgTunnelSocket {

    static final boolean DBG = true;
    private static final boolean VDBG = false;
    private static final String TAG = "QcrilMsgTunnelSocket";
    private static final String mOemIdentifier = "QOEMHOOK";
    public static final String sub_id = "sub_id";
    public static final String pdc_active = "active";
    public static final String pdc_error = "error";
    public static final String audio_state_changed_data = "audio_state_changed_data";

    LocalSocket mSocket;
    HandlerThread mSenderThread;
    QcRilSender mSender;
    Thread mReceiverThread;
    QcRilReceiver mReceiver;
    WakeLock mWakeLock;
    Context mContext;

    int mWakeLockTimeout;
    // The number of requests pending to be sent out, it increases before calling
    // EVENT_SEND and decreases while handling EVENT_SEND. It gets cleared while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesPending;
    // The number of requests sent out but waiting for response. It increases while
    // sending request and decreases while handling response. It should match
    // mRequestList.size() unless there are requests no replied while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesWaiting;

    private Integer mInstanceId;

    // I'd rather this be LinkedList or something
    ArrayList<QcRilRequest> mRequestsList = new ArrayList<QcRilRequest>();

    // ***** Events
    static final int EVENT_SEND = 1;
    static final int EVENT_WAKE_LOCK_TIMEOUT = 2;

    static final int RIL_MAX_COMMAND_BYTES = (8 * 1024);
    static final int RESPONSE_SOLICITED = 0;
    static final int RESPONSE_UNSOLICITED = 1;

    static final String SOCKET_NAME_QCRIL_OEM0 = "qmux_radio/rild_oem0";
    static final String SOCKET_NAME_QCRIL_OEM1 = "qmux_radio/rild_oem1";
    static final String SOCKET_NAME_QCRIL_OEM2 = "qmux_radio/rild_oem2";

    static final int MAX_COUNT_FOR_SILENT_RETRY = 8;

    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;
    /**
     * Wake lock timeout should be longer than the longest timeout in the vendor ril.
     */
    private static final int DEFAULT_WAKE_LOCK_TIMEOUT = 60000;

    protected Registrant mUnsolOemHookRawRegistrant;
    protected Registrant mUnsolOemHookExtAppRegistrant;

    static final String ACTION_SAFE_WIFI_CHANNELS_CHANGED =
            "qualcomm.intent.action.SAFE_WIFI_CHANNELS_CHANGED";
    static final String ACTION_INCREMENTAL_NW_SCAN_IND =
            "qualcomm.intent.action.ACTION_INCREMENTAL_NW_SCAN_IND";
    static final String ACTION_EM_DATA_RECEIVED =
            "qualcomm.intent.action.ACTION_EM_DATA_RECEIVED";
    static final String ACTION_PDC_DATA_RECEIVED =
            "qualcomm.intent.action.ACTION_PDC_DATA_RECEIVED";
    static final String ACTION_PDC_CONFIGS_CLEARED =
            "qualcomm.intent.action.ACTION_PDC_CONFIGS_CLEARED";
    static final String ACTION_AUDIO_STATE_CHANGED =
            "qualcomm.intent.action.ACTION_AUDIO_STATE_CHANGED";
    static final String ACTION_PDC_CONFIGS_VALIDATION =
            "qualcomm.intent.action.ACTION_PDC_CONFIGS_VALIDATION";
    static final String ACTIN_PDC_VALIDATE_DUMPED =
            "qualcomm.intent.action.ACTIN_PDC_VALIDATE_DUMPED";

    public QcrilMsgTunnelSocket(Integer instanceId)
    {
        super();
        mContext = QcrilMsgTunnelService.mContext;

        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.setReferenceCounted(false);
        mWakeLockTimeout = SystemProperties.getInt(TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT,
                DEFAULT_WAKE_LOCK_TIMEOUT);
        mRequestMessagesPending = 0;
        mRequestMessagesWaiting = 0;
        mInstanceId = instanceId;

        Log.v(TAG, "Starting QcRil Sender & Receiver threads");

        mReceiver = new QcRilReceiver();
        mReceiverThread = new Thread(mReceiver, "QcRilReceiver");
        mReceiverThread.start();

        mSenderThread = new HandlerThread("QcRilSender");
        mSenderThread.start();
        Looper looper = mSenderThread.getLooper();
        mSender = new QcRilSender(looper);
    }

    class QcRilSender extends Handler implements Runnable {
        public QcRilSender(Looper looper) {
            super(looper);
        }

        // Only allocated once
        byte[] dataLength = new byte[4];

        // ***** Runnable implementation
        public void
        run() {
            // setup if needed
        }

        // ***** Handler implementation
        @Override
        public void handleMessage(Message msg) {
            QcRilRequest rr = (QcRilRequest) (msg.obj);
            QcRilRequest req = null;

            switch (msg.what) {
                case EVENT_SEND:
                    /**
                     * mRequestMessagePending++ already happened for every EVENT_SEND, thus we
                     * must make sure mRequestMessagePending-- happens once and only once
                     */
                    boolean alreadySubtracted = false;
                    try {
                        LocalSocket s;

                        s = mSocket;

                        if (s == null) {
                            rr.onError(RADIO_NOT_AVAILABLE, null);
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

                        byte[] data;

                        data = rr.mp.marshall();
                        rr.mp.recycle();
                        rr.mp = null;

                        if (data.length > RIL_MAX_COMMAND_BYTES) {
                            throw new RuntimeException(
                                    "Parcel larger than max bytes allowed! "
                                            + data.length);
                        }

                        // parcel length in big endian
                        dataLength[0] = dataLength[1] = 0;
                        dataLength[2] = (byte) ((data.length >> 8) & 0xff);
                        dataLength[3] = (byte) ((data.length) & 0xff);

                        if (VDBG) Log.v(TAG, "writing packet: " + data.length + " bytes");

                        s.getOutputStream().write(dataLength);
                        s.getOutputStream().write(data);
                    } catch (IOException ex) {
                        Log.e(TAG, "IOException", ex);
                        req = findAndRemoveRequestFromList(rr.mSerial);
                        // make sure this request has not already been handled,
                        // eg, if QcRilReceiver cleared the list.
                        if (req != null || !alreadySubtracted) {
                            rr.onError(RADIO_NOT_AVAILABLE, null);
                            rr.release();
                        }
                    } catch (RuntimeException exc) {
                        Log.e(TAG, "Uncaught exception ", exc);
                        req = findAndRemoveRequestFromList(rr.mSerial);
                        // make sure this request has not already been handled,
                        // eg, if QcRilReceiver cleared the list.
                        if (req != null || !alreadySubtracted) {
                            rr.onError(GENERIC_FAILURE, null);
                            rr.release();
                        }
                    } finally {
                        // Note: We are "Done" only if there are no outstanding requests or
                        // replies. Thus this code path will only release the wake lock on errors.
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
                            // The timer of WAKE_LOCK_TIMEOUT is reset with each new send request.
                            // So when WAKE_LOCK_TIMEOUT occurs all requests in mRequestList
                            // already waited at least DEFAULT_WAKE_LOCK_TIMEOUT but no response.
                            // Reset mRequestMessagesWaiting to enable releaseWakeLockIfDone().
                            //
                            // Note: Keep mRequestList so that delayed response
                            // can still be handled when response finally comes.
                            if (mRequestMessagesWaiting != 0) {
                                Log.d(TAG, "NOTE: mReqWaiting is NOT 0 but"
                                        + mRequestMessagesWaiting + " at TIMEOUT, reset!"
                                        + " There still msg waitng for response");

                                mRequestMessagesWaiting = 0;

                                if (DBG) {
                                    synchronized (mRequestsList) {
                                        int count = mRequestsList.size();
                                        Log.d(TAG, "WAKE_LOCK_TIMEOUT " +
                                                " mRequestList=" + count);

                                        for (int i = 0; i < count; i++) {
                                            rr = mRequestsList.get(i);
                                            Log.d(TAG, i + ": [" + rr.mSerial + "] "
                                                    + requestToString(rr.mRequest));
                                        }
                                    }
                                }
                            }
                            // mRequestMessagesPending shows how many requests are waiting to
                            // be sent (and before to be added in request list) since star the
                            // WAKE_LOCK_TIMEOUT timer. Since WAKE_LOCK_TIMEOUT is the expected
                            // time to get response, all requests should already sent out (i.e.
                            // mRequestMessagesPending is 0 )while TIMEOUT occurs.
                            if (mRequestMessagesPending != 0) {
                                Log.e(TAG, "ERROR: mReqPending is NOT 0 but"
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

    class QcRilReceiver implements Runnable {
        byte[] buffer;

        QcRilReceiver() {
            buffer = new byte[RIL_MAX_COMMAND_BYTES];
        }

        public void
        run() {
            int retryCount = 0;
            String rilSocket = SOCKET_NAME_QCRIL_OEM0;

            try {
                for (;;) {
                    LocalSocket s = null;
                    LocalSocketAddress l;

                    // Use 'rild_oem0' socket in case of
                    //  - Non-dsds (mInstanceId will be null)
                    //  - First instance if DSDS and Multi rild enabled
                    //  - DSDS and Multi rild is not enabled.
                    // Use 'rild_oem1' socket for second instance in DSDS and Multi rild enabled
                    if (mInstanceId == 2) {
                        rilSocket = SOCKET_NAME_QCRIL_OEM2;
                    } else if (mInstanceId == 1) {
                        rilSocket = SOCKET_NAME_QCRIL_OEM1;
                    } else {
                        rilSocket = SOCKET_NAME_QCRIL_OEM0;
                    }

                    try {
                        s = new LocalSocket();
                        l = new LocalSocketAddress(rilSocket, LocalSocketAddress.Namespace.RESERVED);
                        s.connect(l);
                        if (VDBG) Log.d(TAG, "Connecting to socket " + s);
                    } catch (IOException ex) {
                        if (isInitialRetry(retryCount)) {
                            Log.e(TAG, "IOException - " + ex + " Reason: " + ex.getMessage());
                        }

                        try {
                            if (s != null) {
                                s.close();
                            }
                        } catch (IOException ex2) {
                            Log.e(TAG, "IOException 2", ex2);
                            // ignore failure to close after failure to connect
                        }

                        // don't print an error message after the the first time
                        // or after the 8th time

                        if (retryCount == MAX_COUNT_FOR_SILENT_RETRY) {
                            Log.e(TAG, "Couldn't find '" + rilSocket + "' socket after "
                                    + retryCount + " times, continuing to retry silently");
                        } else if (isInitialRetry(retryCount)) {
                            Log.i(TAG, "Couldn't find '" + rilSocket + "' socket; " +
                                    "retrying after timeout");
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
                    Log.i(TAG, "Connected to '" + rilSocket + "' socket");

                    int length = 0;
                    try {
                        InputStream is = mSocket.getInputStream();

                        for (;;) {
                            Parcel p;

                            length = readRilMessage(is, buffer);

                            if (length < 0) {
                                // End-of-stream reached
                                break;
                            }

                            p = Parcel.obtain();
                            p.unmarshall(buffer, 0, length);
                            p.setDataPosition(0);

                            if (VDBG) {
                                Log.v(TAG, "Read packet: " + length + " bytes. Data Available = "
                                        + p.dataAvail() + " Position = " + p.dataPosition());
                                p.setDataPosition(0);
                            }

                            processResponse(p);
                            p.recycle();
                        }
                    } catch (java.io.IOException ex) {
                        Log.e(TAG, "'" + rilSocket + "' socket closed", ex);
                    } catch (Throwable tr) {
                        Log.e(TAG, "Uncaught exception read length=" + length +
                                "Exception:" + tr.toString());
                    }

                    Log.i(TAG, "Disconnected from '" + rilSocket + "' socket");

                    try {
                        mSocket.close();
                    } catch (IOException ex) {
                    }

                    mSocket = null;
                    QcRilRequest.resetSerial();

                    // Clear request list on close
                    clearRequestsList(RADIO_NOT_AVAILABLE, false);
                }
            } catch (Throwable tr) {
                Log.e(TAG, "Uncaught exception", tr);
            }

            /* TODO: We're disconnected so we don't know the ril version */
            // notifyRegistrantsQcRilConnectionChanged(-1);
        }
    }

    /**
     * Reads in a single QcRil message off the wire. A QcRil message
     * consists of a 4-byte little-endian length and a subsequent series of
     * bytes. The final message (length header omitted) is read into
     * <code>buffer</code> and the length of the final message (less header) is
     * returned. A return value of -1 indicates end-of-stream.
     *
     * @param is non-null; Stream to read from
     * @param buffer Buffer to fill in. Must be as large as maximum message
     *            size, or an ArrayOutOfBounds exception will be thrown.
     * @return Length of message less header, or -1 on end of stream.
     * @throws IOException
     */
    private static int readRilMessage(InputStream is, byte[] buffer)
            throws IOException {
        int countRead;
        int offset;
        int remaining;
        int messageLength;

        // First, read in the length of the message
        offset = 0;
        remaining = 4;
        countRead = 0;
        if (VDBG) {
            Log.v(TAG, "Before reading offset = " + offset + " remaining = "
                    + remaining + " countRead = " + countRead);
        }
        do {
            countRead = is.read(buffer, offset, remaining);
            if (VDBG) {
                Log.v(TAG, "After reading offset = " + offset + " remaining = "
                        + remaining + " countRead = " + countRead);
            }

            if (countRead < 0) {
                Log.e(TAG, "Hit EOS reading message length");
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        messageLength = ((buffer[0] & 0xff) << 24)
                | ((buffer[1] & 0xff) << 16)
                | ((buffer[2] & 0xff) << 8)
                | (buffer[3] & 0xff);

        if (VDBG) {
            Log.d(TAG, "messageLength extracted from first 4 inputStream reads = " + messageLength);
        }

        // Then, re-use the buffer and read in the message itself
        offset = 0;
        remaining = messageLength;
        countRead = 0;
        if (VDBG) {
            Log.v(TAG, "offset = " + offset + " remaining = " + remaining +
                    " countRead = " + countRead);
        }
        do {
            countRead = is.read(buffer, offset, remaining);
            if (VDBG) {
                Log.v(TAG, "offset = " + offset + " remaining = " + remaining +
                        " countRead = " + countRead);
            }

            if (countRead < 0) {
                Log.e(TAG, "Hit EOS reading message.  messageLength=" + messageLength
                        + " remaining=" + remaining);
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        if (DBG) {
            Log.d(TAG, "readRilMessage: Buffer = " + buffer + " HexData = ["
                    + bytesToHexString(buffer, messageLength) + "]");
        }
        return messageLength;
    }

    private boolean isInitialRetry(int count) {
        return (count > 0 && count < MAX_COUNT_FOR_SILENT_RETRY);
    }

    static String requestToString(int request) {
        switch (request) {
            case RIL_REQUEST_OEM_HOOK_RAW:
                return "OEM_HOOK_RAW";
            case RIL_UNSOL_OEM_HOOK_RAW:
                return "UNSOL_OEM_HOOK_RAW";
            default:
                return "<unknown request>";
        }
    }

    private static String retToString(int req, Object ret) {
        if (ret == null)
            return "";

        StringBuilder sb;
        String s;
        int length;
        if (ret instanceof int[]){
            int[] intArray = (int[]) ret;
            length = intArray.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(intArray[i++]);
                while ( i < length) {
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
                while ( i < length) {
                    sb.append(", ").append(strings[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else {
            s = ret.toString();
        }
        return s;
    }

    private static String bytesToHexString(byte[] bytes, int length) {
        if (bytes == null || length == 0) return null;

        StringBuilder ret = new StringBuilder(2*length);

        for (int i = 0 ; i < length ; i++) {
            int b;

            b = 0x0f & (bytes[i] >> 4);
            ret.append("0123456789abcdef".charAt(b));

            b = 0x0f & bytes[i];
            ret.append("0123456789abcdef".charAt(b));
        }

        return ret.toString();
    }

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

    private QcRilRequest findAndRemoveRequestFromList(int serial) {
        synchronized (mRequestsList) {
            for (int i = 0, s = mRequestsList.size(); i < s; i++) {
                QcRilRequest rr = mRequestsList.get(i);

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

    /**
     * Release each request in mReqeustsList then clear the list
     *
     * @param error is the RIL_Errno sent back
     * @param loggable true means to print all requests in mRequestslist
     */
    private void clearRequestsList(int error, boolean loggable) {
        QcRilRequest rr;
        synchronized (mRequestsList) {
            int count = mRequestsList.size();
            if (DBG && loggable) {
                Log.d(TAG, "WAKE_LOCK_TIMEOUT " +
                        " mReqPending=" + mRequestMessagesPending +
                        " mRequestList=" + count);
            }

            for (int i = 0; i < count; i++) {
                rr = mRequestsList.get(i);
                if (DBG && loggable) {
                    Log.d(TAG, i + ": [" + rr.mSerial + "] " + requestToString(rr.mRequest));
                }
                rr.onError(error, null);
                rr.release();
            }
            mRequestsList.clear();
            mRequestMessagesWaiting = 0;
        }
    }

    private void send(QcRilRequest rr) {
        Message msg;

        if (mSocket == null) {
            rr.onError(RADIO_NOT_AVAILABLE, null);
            rr.release();
            return;
        }

        msg = mSender.obtainMessage(EVENT_SEND, rr);

        acquireWakeLock();

        msg.sendToTarget();
    }

    public void invokeOemRilRequestRaw(byte[] data, Message response) {
        QcRilRequest rr
        = QcRilRequest.obtain(RIL_REQUEST_OEM_HOOK_RAW, response);

        Log.d(TAG, rr.serialString() + " > " + requestToString(rr.mRequest)
                + "[" + bytesToHexString(data, data.length) + "]");

        rr.mp.writeByteArray(data);

        send(rr);

    }

    private Object responseRaw(Parcel p) {
        int num;
        byte response[];

        response = p.createByteArray();

        if (response != null && VDBG) {
            Log.d(TAG, "ByteArray from parcel = " + bytesToHexString(response, response.length));
        }

        return response;
    }

    private void processResponse(Parcel p) {
        int type;

        type = p.readInt();

        if (type == RESPONSE_UNSOLICITED) {
            Log.d(TAG, "Rcvd UNSOL response with " + p.dataAvail() + " bytes data for SUB"
                    + mInstanceId);
            processUnsolicited(p);
        } else if (type == RESPONSE_SOLICITED) {
            Log.d(TAG, "Rcvd SOLICITED response with " + p.dataAvail() + " bytes data for SUB"
                    + mInstanceId);
            processSolicited(p);
        } else {
            Log.e(TAG, "Rcvd UNKNOWN response with " + p.dataAvail() + " bytes data for SUB"
                    + mInstanceId);
        }

        releaseWakeLockIfDone();
    }

    private void processSolicited(Parcel p) {
        int serial, error;
        boolean found = false;

        serial = p.readInt();
        error = p.readInt();

        if (VDBG) {
            Log.d(TAG, "processSolicited. sno: " + serial + " error: " + error);
            Log.d(TAG, "p.dataAvail():" + p.dataAvail() + " Position():" + p.dataPosition());
        }

        QcRilRequest rr;

        rr = findAndRemoveRequestFromList(serial);

        if (rr == null) {
            Log.w(TAG, "Unexpected solicited response! sn: "
                    + serial + " error: " + error);
            return;
        }

        Object ret = null;

        if (error == 0 || p.dataAvail() > 0) {
            // either command succeeds or command fails but with data payload
            try {
                switch (rr.mRequest) {
                    case RIL_REQUEST_OEM_HOOK_RAW:
                        ret = responseRaw(p);
                        if (VDBG) Log.d(TAG, "responseRaw returns: " + ret);
                        break;

                    default:
                        throw new RuntimeException("Unrecognized solicited response: "
                                + rr.mRequest);
                        // break;
                }
            } catch (Throwable tr) {
                // Exceptions here usually mean invalid RIL responses

                Log.e(TAG, rr.serialString() + " < "
                        + requestToString(rr.mRequest)
                        + " exception, possible invalid RIL response", tr);

                if (rr.mResult != null) {
                    AsyncResult.forMessage(rr.mResult, null, tr);
                    rr.mResult.sendToTarget();
                }
                rr.release();
                return;
            }
        }

        if (error != 0) {
            rr.onError(error, ret);
            rr.release();
            return;
        }

        if (ret == null) {
            Log.d(TAG, rr.serialString() + " < " + requestToString(rr.mRequest) + " [null]");
        } else {
            byte[] byteArray = (byte[]) ret;
            int length = byteArray.length;
            Log.d(TAG, rr.serialString() + " < " + requestToString(rr.mRequest) +
                    " [" + bytesToHexString(byteArray, length) + "]");
        }

        if (rr.mResult != null) {
            AsyncResult.forMessage(rr.mResult, ret, null);
            rr.mResult.sendToTarget();
        }

        rr.release();
    }

    private void processUnsolicited(Parcel p) {
        int response;
        Object ret;

        response = p.readInt();

        try {
            switch (response) {
                case RIL_UNSOL_OEM_HOOK_RAW:
                    ret = responseRaw(p);
                    break;
                default:
                    throw new RuntimeException("Unrecognized unsol response: " + response);
                    // break; (implied)
            }
        } catch (Throwable tr) {
            Log.e(TAG, "Exception processing unsol response: " + response +
                       " Exception:" + tr.toString());
            return;
        }

        switch (response) {
            case RIL_UNSOL_OEM_HOOK_RAW:
                if (VDBG) {
                    Log.d(TAG, "Received RIL_UNSOL_OEM_HOOK_RAW message"
                           + Arrays.toString((byte[])ret));
                }

                ByteBuffer oemHookResponse = ByteBuffer.wrap((byte[]) ret);
                oemHookResponse.order(ByteOrder.nativeOrder());

                if (isQcUnsolOemHookResp(oemHookResponse)) {
                    if (VDBG) Log.d(TAG, "OEM ID check Passed");
                    processUnsolOemhookResponse(oemHookResponse);
                } else if (mUnsolOemHookRawRegistrant != null) {
                    Log.d(TAG, "External OEM message, to be notified");
                    mUnsolOemHookRawRegistrant.notifyRegistrant(new AsyncResult(null, ret, null));
                }
                break;
        }
    }

    private boolean isQcUnsolOemHookResp(ByteBuffer oemHookResponse) {
        int INT_SIZE = 4;
        int mHeaderSize = mOemIdentifier.length() + 2 * INT_SIZE;

        /* Check OEM ID in UnsolOemHook response */
        if (oemHookResponse.capacity() < mHeaderSize) {
            /*
             * size of UnsolOemHook message is less than expected, considered as
             * External OEM's message
             */
            Log.d(TAG, "RIL_UNSOL_OEM_HOOK_RAW data size is "
                       + oemHookResponse.capacity()
                    + " assume external OEM message, not QOEMHOOK"
                 );
            return false;
        } else {
            byte[] oem_id_bytes = new byte[mOemIdentifier.length()];
            oemHookResponse.get(oem_id_bytes);
            String oem_id_str = new String(oem_id_bytes);
            if (VDBG) Log.d(TAG, "Oem ID in RIL_UNSOL_OEM_HOOK_RAW is " + oem_id_str);
            if (!oem_id_str.equals(mOemIdentifier)) {
                /* OEM ID not matched, considered as External OEM's message */
                Log.d(TAG, "external OEM message, not QOEMHOOK");
                return false;
            }
        }
        return true;
    }

    private void processUnsolOemhookResponse(ByteBuffer oemHookResponse) {
        /** Starting number for QCRILHOOK request and response IDs */
        final int QCRILHOOK_BASE = 0x80000;

        /** qcrilhook unsolicited response IDs */
        final int QCRILHOOK_UNSOL_WIFI_SAFE_CHANNELS_CHANGED = QCRILHOOK_BASE + 1008;
        final int QCRILHOOK_UNSOL_INCREMENTAL_NW_SCAN_IND = QCRILHOOK_BASE + 1011;
        final int QCRILHOOK_UNSOL_ENGINEER_MODE = QCRILHOOK_BASE + 1012;
        final int QCRILHOOK_UNSOL_PDC_CONFIG = QCRILHOOK_BASE + 1014;
        final int QCRILHOOK_UNSOL_AUDIO_STATE_CHANGED = QCRILHOOK_BASE + 1015;
        final int QCRILHOOK_UNSOL_PDC_CLEAR_CONFIGS = QCRILHOOK_BASE + 1017;
        final int QCRILHOOK_UNSOL_PDC_VALIDATE_CONFIGS = QCRILHOOK_BASE + 1023;
        final int QCRILHOOK_UNSOL_PDC_VALIDATE_DUMPED = QCRILHOOK_BASE + 1024;

        int response_id = 0, response_size = 0;

        response_id = oemHookResponse.getInt();
        if (VDBG) Log.d(TAG, "Response ID in RIL_UNSOL_OEM_HOOK_RAW is " + response_id);

        response_size = oemHookResponse.getInt();
        if (response_size < 0 || response_size > RIL_MAX_COMMAND_BYTES) {
            Log.e(TAG, "Response Size is Invalid " + response_size);
            return;
        }
        byte[] response_data = new byte[response_size];
        oemHookResponse.get(response_data, 0, response_size);

        switch (response_id) {
            case QCRILHOOK_UNSOL_WIFI_SAFE_CHANNELS_CHANGED:
                broadcastWifiChannelsChangedIntent(response_data);
                break;
            case QCRILHOOK_UNSOL_INCREMENTAL_NW_SCAN_IND:
                broadcastIncrNwScanInd(response_data);
                break;
            case QCRILHOOK_UNSOL_ENGINEER_MODE:
                broadcastEngineerMode(response_data);
                break;
            case QCRILHOOK_UNSOL_PDC_CONFIG:
                broadcastDeviceConfig(response_data);
                break;
            case QCRILHOOK_UNSOL_PDC_CLEAR_CONFIGS:
                broadcastClearConfigs(response_data);
                break;
            case QCRILHOOK_UNSOL_AUDIO_STATE_CHANGED:
                broadcastAudioStateChanged(response_data);
                break;
            case QCRILHOOK_UNSOL_PDC_VALIDATE_CONFIGS:
                broadcastValidateConfigs(response_data);
                break;
            case QCRILHOOK_UNSOL_PDC_VALIDATE_DUMPED:
                broadcastValidateDumped(response_data);
                break;
            default:
                Log.d(TAG, "Response ID " + response_id + " is not served in this process.");
                Log.d(TAG, "To broadcast an Intent via the notifier to external apps");
                if (mUnsolOemHookExtAppRegistrant != null) {
                    oemHookResponse.rewind();
                    byte[] origData = oemHookResponse.array();
                    mUnsolOemHookExtAppRegistrant.notifyRegistrant(new AsyncResult(null, origData,
                            null));
                }
                break;
        }
    }

    private void broadcastWifiChannelsChangedIntent(byte[] data) {
        Intent intent = new Intent(ACTION_SAFE_WIFI_CHANNELS_CHANGED);
        Log.d(TAG, "WifiSafeChannels " + Arrays.toString(data));

        String s;
        try {
            s = new String(data, "US-ASCII");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Decoding failed: " + e);
            return;
        }

        Log.d(TAG, "Decoded string " + s);
        // Channels info is divided by commas
        String[] channels = s.split(",");
        Log.d(TAG, "Parsed channels " + Arrays.toString(channels));
        intent.putExtra("current_channel", Integer.parseInt(channels[0]));
        intent.putExtra("start_safe_channel", Integer.parseInt(channels[1]));
        intent.putExtra("end_safe_channel", Integer.parseInt(channels[2]));

        Log.d(TAG, "Broadcasting intent ACTION_SAFE_WIFI_CHANNELS_CHANGED ");
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE,
                UserHandle.USER_ALL);
    }

    private void broadcastIncrNwScanInd(byte[] data) {
        Log.d(TAG, "Incremental nw scan data " + Arrays.toString(data));
        ByteBuffer payload = ByteBuffer.wrap(data);
        payload.order(ByteOrder.nativeOrder());

        int scanResult = payload.get();
        // Each set of data has 4 strings, qcril sends number of sets
        // so multiply by 4 to get number of strings
        int numOfStrings = payload.get() * 4;
        Log.d(TAG, "scanResult =" + scanResult + "numOfStrings = " + numOfStrings);
        if (numOfStrings < 0) {
            Log.d(TAG, "Invalid number of strings" + numOfStrings);
            return;
        }
        String nwScanInfo[] = new String[numOfStrings];
        for (int i = 0; i < numOfStrings; i++) {
            short stringLen = payload.getShort();
            Log.d(TAG, "stringLen =" + stringLen);
            byte bytes[] = new byte[stringLen];
            payload.get(bytes);
            nwScanInfo[i] = new String(bytes);
            Log.d(TAG, "i = " + i + "String is " + nwScanInfo[i]);
        }
        Intent intent = new Intent(ACTION_INCREMENTAL_NW_SCAN_IND);
        intent.putExtra("scan_result", scanResult);
        intent.putExtra("incr_nw_scan_data", nwScanInfo);
        intent.putExtra("sub_id", mInstanceId);
        mContext.sendBroadcast(intent);
    }

    private void broadcastValidateDumped(byte[] data) {
        Intent intent = new Intent(ACTIN_PDC_VALIDATE_DUMPED);
        Log.d(TAG, "PDC Validate Dumped " + Arrays.toString(data));
        intent.putExtra("dump_file", data);

        Log.d(TAG, "Broadcasting intent ACTION_PDC_VALIDATE_DUMPED");
        mContext.sendBroadcast(intent);
    }

    private void broadcastValidateConfigs(byte[] data) {
        Log.d(TAG, "PDC Validate Configs " + Arrays.toString(data));
        ByteBuffer payload = ByteBuffer.wrap(data);
        payload.order(ByteOrder.nativeOrder());

        // get the result
        int result = payload.getInt();
        // get the index
        int index = payload.getInt();
        // get the nv item len
        int nvItemLen = payload.getInt();
        // get the ref valalue len
        int nvRefValLen = payload.getInt();
        // get the device value len
        int nvCurValLen= payload.getInt();

        Intent intent = new Intent(ACTION_PDC_CONFIGS_VALIDATION);
        intent.putExtra("result", result);
        intent.putExtra("index", index);
        Log.d(TAG, "result:" + result +  " index:" + index);
        if ((nvItemLen != 0) && (result == 0)) {
            // get NV item Info
            byte bytes[] = new byte[nvItemLen];
            payload.get(bytes);
            String nvItemInfo = new String(bytes);
            Log.d(TAG, "nvItemInfo:" + nvItemInfo);
            intent.putExtra("nv_item", nvItemInfo);

            // get Ref value
            if (nvRefValLen == 0) {
                intent.putExtra("nv_item", "");
            } else {
                bytes = new byte[nvRefValLen];
                payload.get(bytes);
                String nvRefVal = new String(bytes);
                Log.d(TAG, "nvRefVal:" + nvRefVal);
                intent.putExtra("ref_value", nvRefVal);
            }

            // get Device Value
            if (nvCurValLen == 0) {
                 intent.putExtra("cur_value", "");
            } else {
                bytes = new byte[nvCurValLen];
                payload.get(bytes);
                String nvCurVal = new String(bytes);
                Log.d(TAG, "nvCurVal:" + nvCurVal);
                intent.putExtra("cur_value", nvCurVal);
            }
        }
        // broadcast result
        mContext.sendBroadcast(intent);
    }

    private void broadcastEngineerMode(byte[] data) {
        Intent intent = new Intent(ACTION_EM_DATA_RECEIVED);
        Log.d(TAG, "EM data: " + Arrays.toString(data));
        intent.putExtra("sub_id", mInstanceId);
        intent.putExtra("em_data", data);

        Log.d(TAG, "Broadcasting intent ACTION_EM_DATA_RECEIVED");
        mContext.sendBroadcast(intent);
    }

    private void broadcastDeviceConfig(byte[] data) {
        Intent intent = new Intent(ACTION_PDC_DATA_RECEIVED);
        Log.d(TAG, "DeviceConfig (PDC) data: " + Arrays.toString(data));
        intent.putExtra(sub_id, mInstanceId);
        intent.putExtra(pdc_active, data);
        intent.putExtra(pdc_error, 0);

        Log.d(TAG, "Broadcasting intent ACTION_PDC_DATA_RECEIVED");
        mContext.sendBroadcast(intent);
    }

    private void broadcastClearConfigs(byte[] data) {
        Intent intent = new Intent(ACTION_PDC_CONFIGS_CLEARED);
        Log.d(TAG, "ClearConfig (PDC) data: " + Arrays.toString(data));
        intent.putExtra(sub_id, mInstanceId);
        intent.putExtra(pdc_active, data);
        intent.putExtra(pdc_error, 0);

        Log.d(TAG, "Broadcasting intent ACTION_PDC_CONFIGS_CLEARED");
        mContext.sendBroadcast(intent);
    }

    private void broadcastAudioStateChanged(byte[] data) {
        Intent intent = new Intent(ACTION_AUDIO_STATE_CHANGED);
        Log.d(TAG, "AudioState data received: " + new String(data));
        intent.putExtra(audio_state_changed_data, data);

        Log.d(TAG, "Broadcasting intent ACTION_AUDIO_STATE_CHANGED");
        mContext.sendBroadcast(intent);
    }

    public void setOnUnsolOemHookRaw(Handler h, int what, Object obj) {
        mUnsolOemHookRawRegistrant = new Registrant(h, what, obj);
    }

    public void unSetOnUnsolOemHookRaw(Handler h) {
        mUnsolOemHookRawRegistrant.clear();
    }

    public void setOnUnsolOemHookExtApp(Handler h, int what, Object obj) {
        mUnsolOemHookExtAppRegistrant = new Registrant(h, what, obj);
    }

    public void unSetOnUnsolOemHookExtApp(Handler h) {
        mUnsolOemHookExtAppRegistrant.clear();
    }
}

class QcRilRequest {
    static final String TAG = "QcRilRequest";

    // ***** Class Variables
    static int sNextSerial = 1000;
    static Object sSerialMonitor = new Object();
    private static Object sPoolSync = new Object();
    private static QcRilRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    // ***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    Parcel mp;
    QcRilRequest mNext;

    /**
     * Retrieves a new QcRilRequest instance from the pool.
     *
     * @param request RIL_REQUEST_*
     * @param result sent when operation completes
     * @return a RILRequest instance from the pool.
     */
    static QcRilRequest obtain(int request, Message result) {
        QcRilRequest rr = null;

        synchronized (sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new QcRilRequest();
        }

        synchronized (sSerialMonitor) {
            rr.mSerial = sNextSerial++;
        }
        rr.mRequest = request;
        rr.mResult = result;
        rr.mp = Parcel.obtain();

        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }

        // first elements in any QcRil Parcel
        rr.mp.writeInt(request);
        rr.mp.writeInt(rr.mSerial);

        return rr;
    }

    /**
     * Returns a QcRilRequest instance to the pool. Note: This should only be
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

    private QcRilRequest() {
    }

    static void resetSerial() {
        synchronized (sSerialMonitor) {
            sNextSerial = 1000;
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
        CommandException ex;

        ex = CommandException.fromRilErrno(error);

        if (QcrilMsgTunnelSocket.DBG)
            Log.d(TAG, serialString() + " < "
                    + QcrilMsgTunnelSocket.requestToString(mRequest)
                    + " error: " + ex);

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }

        if (mp != null) {
            mp.recycle();
            mp = null;
        }
    }
}
