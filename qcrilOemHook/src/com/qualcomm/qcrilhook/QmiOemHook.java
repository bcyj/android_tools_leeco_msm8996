/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.BaseQmiTypes.QmiBase;
import com.qualcomm.qcrilhook.QmiOemHookConstants.ResponseType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiNull;

public class QmiOemHook extends Handler {

    private QcRilHook mQcRilOemHook;

    private static String LOG_TAG = "QMI_OEMHOOK";

    public static HashMap<Short, Registrant> serviceRegistrantsMap;
    static {
        serviceRegistrantsMap = new HashMap<Short, Registrant>();
    }

    private static RegistrantList sReadyCbRegistrantList = new RegistrantList();

    private static final int RESERVED_SIZE = 8;

    private static final boolean enableVLog = true;

    private static final int QMI_OEM_HOOK_UNSOL = 0;

    private static final int DEFAULT_PHONE = 0;

    private static QmiOemHook mInstance;

    private Context mContext;

    private static int mRefCount = 0;

    int mResponseResult = 0;

    public ByteBuffer respByteBuf;

    private static boolean mIsServiceConnected = false;

    /**
     * Called when connection to QcrilMsgTunnelService has been established.
     */
    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            mIsServiceConnected = true;
            AsyncResult ar = new AsyncResult(null, mIsServiceConnected, null);
            Log.i(LOG_TAG, "onQcRilHookReadyCb notifying registrants");
            sReadyCbRegistrantList.notifyRegistrants(ar);
        }

        public synchronized void onQcRilHookDisconnected() {
            mIsServiceConnected = false;
            AsyncResult ar = new AsyncResult(null, mIsServiceConnected, null);
            Log.i(LOG_TAG, "onQcRilHookReadyCb: service disconnected; notifying registrants.");
                    sReadyCbRegistrantList.notifyRegistrants(ar);
        }
    };

    /**
     * Registers for any OEM_HOOK_UNSOLs as soon as this class is instantiated
     * Passes the application context to QcRilHook to register BroadcastReceiver
     */
    private QmiOemHook(Context context) {
        mQcRilOemHook = new QcRilHook(context, mQcrilHookCb);
        QcRilHook.register(this, QMI_OEM_HOOK_UNSOL, null);
    }

    private QmiOemHook(Context context, Looper looper) {
        super(looper);
        mQcRilOemHook = new QcRilHook(context, mQcrilHookCb);
        QcRilHook.register(this, QMI_OEM_HOOK_UNSOL, null);

    }


    public static synchronized QmiOemHook getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new QmiOemHook(context);
        }
        mRefCount++;
        return mInstance;
    }

    /* This method is introduced for such application who wants to handle unsol
     * indications on a thread different from the one on which request was made.
     *
     * Such application should create a background thread and pass the
     * handler. Wrapper API would extract the looper from that handler and pass
     * it to OEM Hook library.
     *
     * OEM Hook library instantiate a handler and due to this looper,
     * the handler would get associated with the background thread.
     *
     * OEM Hook library also instantiates a broadcast listener for unsol events
     * from telephony framework. In case of any such event the handler of
     * library is invoked which delegates the processing to the handler suppied
     * by application and rest of processing of the event is handled in that
     * handler(and the associated) thread context.
     *
     */
    public static synchronized QmiOemHook getInstance(Context context, Looper looper) {
        if (mInstance == null) {
            mInstance = new QmiOemHook(context, looper);
        }

        mRefCount++;
        return mInstance;
    }

    public synchronized void dispose() {
        mRefCount--;

        if (mRefCount == 0) {
            vLog("dispose(): Unregistering QcRilHook and calling QcRilHook dispose");
            QcRilHook.unregister(this);
            mIsServiceConnected = false;
            mQcRilOemHook.dispose();
            mInstance = null;
            sReadyCbRegistrantList.removeCleared();
        } else {
            vLog("dispose mRefCount = " + mRefCount);
        }
    }

    // IO Functions
    private void vLog(String logString) {
        if (enableVLog) {
            Log.v(LOG_TAG, logString);
        }
    }

    /**
     * Registers different services so that later responses can be parsed and
     * depending on the serviceId, appropriate service can be notified The App
     * can use instanceOf operator to figure out the which Unsol Object is
     * received
     */
    public static void registerService(short serviceId, Handler h, int what) {
        Log.v(LOG_TAG, "Registering Service Id = " + serviceId + " h = " + h + " what = " + what);
        synchronized (serviceRegistrantsMap) {
            serviceRegistrantsMap.put(serviceId, new Registrant(h, what, null));
        }
    }

    /**
     * Registers different services for receiving a Callback when QcRilHook is ready i.e when
     * the service has successfully bound to QcrilMsgTunnelService.
     */
    public static void registerOnReadyCb(Handler h, int what, Object obj) {
        Log.v(LOG_TAG, "Registering Service for OnQcRilHookReadyCb = " + " h = " + h
                + " what = " + what);
        synchronized (sReadyCbRegistrantList) {
            sReadyCbRegistrantList.add(new Registrant(h, what, obj));
        }
    }

    public static void unregisterService(int serviceId) {
        synchronized (serviceRegistrantsMap) {
            serviceRegistrantsMap.remove(serviceId);
        }
    }

    public static void unregisterOnReadyCb(Handler h) {
        synchronized (sReadyCbRegistrantList) {
            sReadyCbRegistrantList.remove(h);
        }
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case QMI_OEM_HOOK_UNSOL:
                Log.v(LOG_TAG, "Thread="+Thread.currentThread().getName()+" received "+msg);
                Log.v(LOG_TAG, "QMI_OEM_HOOK_UNSOL received");
                AsyncResult ar = (AsyncResult) msg.obj;
                Message mesg = (Message) ar.result;
                byte[] response = (byte[]) mesg.obj;
                int phoneId = mesg.arg1;
                Log.d(LOG_TAG, "QMI_OEM_HOOK_UNSOL received phoneId: " + phoneId);
                receive(response, null, QmiOemHookConstants.ResponseType.IS_UNSOL, phoneId);
        }
    }

    /**
     * Filters out the values from the response byte stream. Response byte
     * stream structure: ResponseID(4) | ResponseSize(4) | ServiceID(2) |
     * MsgID(2) | Error(2) | T(1) | L(2) | V(L)
     */
    public static HashMap<Integer, Object> receive(byte[] payload, Message msg,
            QmiOemHookConstants.ResponseType responseType, int phoneId) {
        Log.v(LOG_TAG, "receive responseData = " + EmbmsOemHook.bytesToHexString(payload)
                + " message=" + msg + " responseType= " + responseType);
        ByteBuffer respByteBuf = ByteBuffer.wrap(payload);

        if (respByteBuf == null) {
            Log.v(LOG_TAG, "respByteBuf is null");
            return null;
        }
        respByteBuf.order(BaseQmiItemType.QMI_BYTE_ORDER);

        Log.v(LOG_TAG, "receive respByteBuf after ByteBuffer.wrap(payload) = "
                + EmbmsOemHook.bytesToHexString(respByteBuf.array()));
        Log.v(LOG_TAG, "receive respByteBuf = " + respByteBuf);

        int requestId = respByteBuf.getInt();
        int responseSize = respByteBuf.getInt();
        short serviceId = -1;
        int successStatus = -1;

        if (!isValidQmiMessage(responseType, requestId)) {
            Log.e(LOG_TAG, "requestId NOT in QMI OemHook range, No further processing");
            return null;
        }

        if (responseSize > 0) {
            serviceId = respByteBuf.getShort();
            short messageId = respByteBuf.getShort();

            // ServiceID(2 bytes) + MsgId(2bytes) needs to be excluded
            int responseTlvSize = responseSize - 4;
            if (responseType != QmiOemHookConstants.ResponseType.IS_UNSOL) {
                // Assuming 2 bytes short indicates error code
                successStatus = PrimitiveParser.toUnsigned(respByteBuf.getShort());

                responseTlvSize -= QmiOemHookConstants.SHORT_SIZE;
            }

            Log.d(LOG_TAG, "receive requestId=" + requestId + " responseSize=" + responseSize
                    + " responseTlvSize=" + responseTlvSize + " serviceId=" + serviceId
                    + " messageId=" + messageId + " successStatus = " + successStatus
                    + " phoneId: "+ phoneId);

            HashMap<Integer, Object> hashMap = new HashMap<Integer, Object>();
            hashMap.put(QmiOemHookConstants.REQUEST_ID, requestId);
            hashMap.put(QmiOemHookConstants.RESPONSE_TLV_SIZE, responseTlvSize);
            hashMap.put(QmiOemHookConstants.SERVICE_ID, serviceId);
            hashMap.put(QmiOemHookConstants.MESSAGE_ID, messageId);
            hashMap.put(QmiOemHookConstants.SUCCESS_STATUS, successStatus);
            hashMap.put(QmiOemHookConstants.MESSAGE, msg);
            hashMap.put(QmiOemHookConstants.RESPONSE_TYPE, responseType);
            hashMap.put(QmiOemHookConstants.RESPONSE_BUFFER, respByteBuf);
            hashMap.put(QmiOemHookConstants.PHONE_ID, phoneId);

            if (responseType == QmiOemHookConstants.ResponseType.IS_UNSOL
                    || responseType == QmiOemHookConstants.ResponseType.IS_ASYNC_RESPONSE) {
                AsyncResult ar = new AsyncResult(null, hashMap, null);
                Registrant r = serviceRegistrantsMap.get(serviceId);
                if (r != null) {
                    Log.v(LOG_TAG, "Notifying registrant for responseType = " + responseType);
                    r.notifyRegistrant(ar);
                    return null;
                } else {
                    Log.e(LOG_TAG, "Did not find the registered serviceId = " + serviceId);
                }
            } else {
                return hashMap;
            }
        }
        return null;
    }

    private static boolean isValidQmiMessage(ResponseType responseType, int requestId) {
        if (responseType == QmiOemHookConstants.ResponseType.IS_UNSOL) {
            return (requestId == QcRilHook.QCRILHOOK_UNSOL_OEMHOOK);
        }
        // solicited (SYNC/ASYNC) responses fall down here
        return (requestId == QcRilHook.QCRILHOOK_QMI_OEMHOOK_REQUEST_ID);
    }

    /*
     * createPayload Format: | Reserved(8) | ServiceId(2) | MessageId(2) | T(1)|
     * L(2) | V(L)|
     */
    private byte[] createPayload(short serviceId, short messageId, short[] types,
            BaseQmiItemType[] qmiItems) {
        int tlvSize = 0;
        if (qmiItems == null || types == null || qmiItems[0] == null ) {
            Log.v(LOG_TAG, "This message has no payload");
        } else {

            for (int i = 0; i < qmiItems.length; i++) {
                tlvSize += BaseQmiItemType.TLV_TYPE_SIZE + BaseQmiItemType.TLV_LENGTH_SIZE
                        + qmiItems[i].getSize();
            }
        }

        // Reserved/modem id (8 bytes) + SERVICE_ID_SIZE + MESSAGE_ID_SIZE +
        // TLVs (tlvSize bytes)
        ByteBuffer buf = QmiBase.createByteBuffer(RESERVED_SIZE
                + QmiOemHookConstants.SERVICE_ID_SIZE + QmiOemHookConstants.MESSAGE_ID_SIZE
                + tlvSize);
        // Fill the reserved field with 0s
        buf.putInt(0);
        buf.putInt(0);

        buf.putShort(serviceId);
        buf.putShort(messageId);
        Log.v(LOG_TAG, "createPayload: serviceId= " + serviceId + " messageId= " + messageId);
        if (qmiItems != null && types != null && qmiItems[0] != null) {
            for (int i = 0; i < qmiItems.length; i++) {
                vLog(qmiItems[i].toString());
                buf.put(qmiItems[i].toTlv(types[i]));
                Log.v(LOG_TAG, "Intermediate buf in QmiOemHook sendQmiMessage Sync or Async = "
                        + EmbmsOemHook.bytesToHexString(qmiItems[i].toTlv(types[i])));
            }
        }

        Log.v(LOG_TAG, "Byte buf in QmiOemHook createPayload = " + buf);
        return buf.array();
    }

    /**
     * @deprecated Used by QmiNvItems only at present. To be removed once new
     *             byte stream structures are adopted in RIL side Logging for
     *             all inputed QmiItems occurs within this function, so no
     *             additional input logging is required. Logging of returned
     *             values, however, must be handled by individual functions.
     */
    public byte[] sendQmiMessage(int serviceHook, short[] types, BaseQmiItemType[] qmiItems)
            throws IOException {
        int msgSize = 0;
        int HEADER_SIZE = 4;
        int modemId = 0;
        for (int i = 0; i < qmiItems.length; i++) {
            msgSize += BaseQmiItemType.TLV_TYPE_SIZE + BaseQmiItemType.TLV_LENGTH_SIZE
                    + qmiItems[i].getSize();
        }

        /*
         * TODO: This will need to be changed as per Android RIL IDL
         * requirements/new bytestream structures
         */
        ByteBuffer buf = QmiBase.createByteBuffer(HEADER_SIZE + msgSize);
        buf.putInt(modemId);
        buf.putShort(PrimitiveParser.parseShort(msgSize));

        for (int i = 0; i < qmiItems.length; i++) {
            vLog(qmiItems[i].toString());
            buf.put(qmiItems[i].toTlv(types[i]));
        }
        AsyncResult result = mQcRilOemHook.sendQcRilHookMsg(serviceHook, buf.array());

        if (result.exception != null) {
            Log.w(LOG_TAG, String.format("sendQmiMessage() Failed : %s", result.exception
                    .toString()));
            result.exception.printStackTrace();
            throw new IOException();
        }
        return (byte[]) result.result;
    }

    public HashMap<Integer, Object> sendQmiMessageSync(short serviceId, short messageId,
            short[] types, BaseQmiItemType[] qmiItems) throws IOException {
        return sendQmiMessageSync(serviceId, messageId, types, qmiItems, DEFAULT_PHONE);
    }
    /**
     * Synchronous method to create TLVs and parse the response received
     * appropriately Request Format: | "QOEMHOOK"(8) | RequestID(4) |
     * RequestSize(4) | Payload |
     *
     * @param types
     * @param qmiItems
     * @param msg Message from the application. Response is decoded by receive
     *            method and appropriate handler is called
     * @throws IOException
     */
    public HashMap<Integer, Object> sendQmiMessageSync(short serviceId, short messageId,
            short[] types, BaseQmiItemType[] qmiItems, int phoneId) throws IOException {

        AsyncResult result = mQcRilOemHook.sendQcRilHookMsg(
                IQcRilHook.QCRILHOOK_QMI_OEMHOOK_REQUEST_ID,
                createPayload(serviceId, messageId, types, qmiItems), phoneId);

        if (result.exception != null) {
            Log.w(LOG_TAG, String.format("sendQmiMessage() Failed : %s", result.exception
                    .toString()));
            result.exception.printStackTrace();
            throw new IOException();
        }
        byte[] responseData = (byte[]) result.result;

        return receive(responseData, null, QmiOemHookConstants.ResponseType.IS_SYNC_RESPONSE,
                phoneId);
    }

    /**
     * Asynchronous method to create TLVs and parse the response received
     * appropriately and Application's msg should be used to notify the App
     * Request Format: | "QOEMHOOK"(8) | RequestID(4) | RequestSize(4) | Payload
     * |
     *
     * @param types
     * @param qmiItems
     * @param msg This is the Message object sent by Application
     * @throws IOException
     */
    public void sendQmiMessageAsync(short serviceId, short messageId, short[] types,
            BaseQmiItemType[] qmiItems, Message msg) throws IOException {
        sendQmiMessageAsync(serviceId, messageId, types, qmiItems, msg, DEFAULT_PHONE);
    }

    /**
     * Asynchronous method to create TLVs and parse the response received
     * appropriately and Application's msg should be used to notify the App
     * Request Format: | "QOEMHOOK"(8) | RequestID(4) | RequestSize(4) | Payload
     * |
     *
     * @param types
     * @param qmiItems
     * @param msg This is the Message object sent by Application
     * @throws IOException
     */
    public void sendQmiMessageAsync(short serviceId, short messageId, short[] types,
            BaseQmiItemType[] qmiItems, Message msg, int phoneId) throws IOException {
        Log.w(LOG_TAG, "sendQmiMessageAsync phoneId: "+ phoneId);
        OemHookCallback qmiOemHookCb = new OemHookCallback(msg);
        mQcRilOemHook.sendQcRilHookMsgAsync(IQcRilHook.QCRILHOOK_QMI_OEMHOOK_REQUEST_ID,
                createPayload(
                        serviceId, messageId, types, qmiItems), qmiOemHookCb, phoneId);
    }

    /**
     * @deprecated Used by QmiNvItems only at present. To be removed once new
     *             byte stream structures are adopted in RIL side
     * @param serviceHook
     * @param type
     * @param qmiItem
     * @return
     * @throws IOException
     */
    public byte[] sendQmiMessage(int serviceHook, short type, BaseQmiItemType qmiItem)
            throws IOException {
        return sendQmiMessage(serviceHook, new short[] {
                type
        }, new BaseQmiItemType[] {
                qmiItem
        });
    }

    public HashMap<Integer, Object> sendQmiMessageSync(short serviceId, short messageId,
            short type, BaseQmiItemType qmiItem) throws IOException {
        return sendQmiMessageSync(serviceId, messageId, type, qmiItem, DEFAULT_PHONE);
    }

    public HashMap<Integer, Object> sendQmiMessageSync(short serviceId, short messageId,
            short type, BaseQmiItemType qmiItem, int phoneId) throws IOException {
        return sendQmiMessageSync(serviceId, messageId, new short[] {
                type
        }, new BaseQmiItemType[] {
                qmiItem
        }, phoneId);
    }

    public void sendQmiMessageAsync(short serviceId, short messageId, short type,
            BaseQmiItemType qmiItem, Message msg) throws IOException {
        sendQmiMessageAsync(serviceId, messageId, type, qmiItem, msg, DEFAULT_PHONE);
    }

    public void sendQmiMessageAsync(short serviceId, short messageId, short type,
            BaseQmiItemType qmiItem, Message msg, int phoneId) throws IOException {
        Log.w(LOG_TAG, "sendQmiMessageAsync phoneId: "+ phoneId);
        sendQmiMessageAsync(serviceId, messageId, new short[] {
                type
        }, new BaseQmiItemType[] {
                qmiItem
        }, msg, phoneId);
    }

    /**
     * @deprecated Used by QmiNvItems only at present. To be removed once new
     *             byte stream structures are adopted in RIL side
     * @param serviceHook
     * @return
     * @throws IOException
     */
    public byte[] sendQmiMessage(int serviceHook) throws IOException {
        // TODO: Change it to send null as TLV is ignored for requests
        // with no parameters
        return sendQmiMessage(serviceHook, (short) 0, new QmiNull());
    }

    public HashMap<Integer, Object> sendQmiMessageSync(short serviceId, short messageId)
            throws IOException {
        return sendQmiMessageSync(serviceId, messageId, null, null);

    }

    public void sendQmiMessageAsync(short serviceId, short messageId, Message msg)
            throws IOException {
        sendQmiMessageAsync(serviceId, messageId, msg, DEFAULT_PHONE);
    }

    public void sendQmiMessageAsync(short serviceId, short messageId, Message msg, int phoneId)
            throws IOException {
        sendQmiMessageAsync(serviceId, messageId, null, null, msg, phoneId);;
    }


    protected void finalize() {
        Log.v(LOG_TAG, "is destroyed");
    }
}
