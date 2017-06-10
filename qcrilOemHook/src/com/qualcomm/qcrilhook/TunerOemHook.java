/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiStructType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiArray;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiByte;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiInteger;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiNull;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiString;


public class TunerOemHook  {
    private static String LOG_TAG = "TunerOemHook";

    private static final short TUNER_SERVICE_ID = 0x04;

    // Message IDs from IDL - qtuner_v01
    public static final short QCRILHOOK_TUNER_RFRPE_SET_RFM_SCENARIO_REQ = 0x0020;
    public static final short QCRILHOOK_TUNER_RFRPE_GET_RFM_SCENARIO_REQ = 0x0021;
    public static final short QCRILHOOK_TUNER_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ = 0x0022;

    private static final byte TLV_TYPE_COMMON_REQ_SCENARIO_ID = 0x01;
    private static final byte TLV_TYPE_GET_PROVISION_TABLE_OPTIONAL_TAG1 = 0x10;
    private static final byte TLV_TYPE_GET_PROVISION_TABLE_OPTIONAL_TAG2 = 0x11;

    /* PACKAGE SCOPE */
    Context mContext;

    /* PRIVATE SCOPE */
    private QmiOemHook mQmiOemHook;
    private static int mRefCount = 0;
    private static TunerOemHook mInstance;

    /* This private constructor associates the listenerLooper with
     * OEM HOOK library's broadcast receiver. So the message would
     * be received by main thread during execution of broadcast
     * receiver and then it would be delegated to client's listener
     * thread for background parsing and any other heavy work, later
     * this background thread may choose to inform main thread for
     * any UI update.
     */
    private TunerOemHook(Context context, Looper listenerLooper) {
        mContext = context;
        mQmiOemHook = QmiOemHook.getInstance(context, listenerLooper);
    }

    /**
     * Application should use this the first time it is invoked and pass the
     * context so that registrations for Unsols are carried out.
     *
     * First time invocation would result in extracting the looper from the
     * listernerHandler and a call to private constructor.
     *
     * @param context
     * @return
     */
    public static TunerOemHook getInstance(Context context, Handler listenerHandler) {
        if (mInstance == null) {
            mInstance = new TunerOemHook(context, listenerHandler.getLooper());
        } else {
            mInstance.mContext = context;
        }
        mRefCount++;
        return mInstance;
    }

    public synchronized void registerOnReadyCb(Handler h, int what, Object obj) {
        QmiOemHook.registerOnReadyCb(h, what, null);
    }

    public synchronized void unregisterOnReadyCb(Handler h) {
        QmiOemHook.unregisterOnReadyCb(h);
    }

    public synchronized void dispose() {
        mRefCount--;
        if (mRefCount == 0) {
            Log.v(LOG_TAG, "dispose(): Unregistering service");
            mQmiOemHook.dispose();
            mQmiOemHook = null;
            mInstance = null;
        } else {
            Log.v(LOG_TAG, "dispose mRefCount = " + mRefCount);
        }
    }

    public Integer tuner_send_proximity_updates(int[] proximityValues) {
        ScenarioRequest req = new ScenarioRequest(proximityValues);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    TUNER_SERVICE_ID,
                    QCRILHOOK_TUNER_RFRPE_SET_RFM_SCENARIO_REQ,
                    req.getTypes(),
                    req.getItems());
            return  (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }

    }

    public int tuner_get_provisioned_table_revision() {
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    TUNER_SERVICE_ID,
                    QCRILHOOK_TUNER_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ);
            return  (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return -1;
        }

    }

    static public class TunerUnsolIndication {
        public int oemHookMesgId;
        public Object obj;
    }

    static public class TunerSolResponse {
        public int result;
        public Object data;
    }

    /**
     * Parses all the responses - sync, async, unsol and returns Object which
     * needs to be cast back to the required type
     *
     * @param map
     * @return Object
     */
    static public Object receive(HashMap<Integer, Object> map) {

        int requestId = (Integer) map.get(QmiOemHookConstants.REQUEST_ID);
        int responseSize = (Integer) map.get(QmiOemHookConstants.RESPONSE_TLV_SIZE);
        int successStatus = (Integer) map.get(QmiOemHookConstants.SUCCESS_STATUS);
        short messageId = (Short) map.get(QmiOemHookConstants.MESSAGE_ID);

        QmiOemHookConstants.ResponseType respType = (QmiOemHookConstants.ResponseType) map
            .get(QmiOemHookConstants.RESPONSE_TYPE);

        Message msg = (Message) map.get(QmiOemHookConstants.MESSAGE);
        ByteBuffer respByteBuf = (ByteBuffer) map.get(QmiOemHookConstants.RESPONSE_BUFFER);

        Log.v(LOG_TAG, "receive respByteBuf = " + respByteBuf);
        Log.v(LOG_TAG, " responseSize=" + responseSize + " successStatus=" + successStatus
                + " messageId= " + messageId);

        Object returnObject = successStatus;

        switch (messageId) {
            case QCRILHOOK_TUNER_RFRPE_SET_RFM_SCENARIO_REQ:
            {
                Log.v(LOG_TAG, "Response: QCRILHOOK_TUNER_RFRPE_SET_RFM_SCENARIO_REQ="
                        + successStatus);

                break;
            }
            case QCRILHOOK_TUNER_RFRPE_GET_RFM_SCENARIO_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_TUNER_RFRPE_GET_RFM_SCENARIO_REQ="
                        + successStatus);

                break;
            }
            case QCRILHOOK_TUNER_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_TUNER_RFRPE_GET_PROVISIONED_TABLE_REVISION_REQ="
                        + successStatus);
                ProvisionTable info = new ProvisionTable(respByteBuf);
                returnObject = info.prv_tbl_rev; // for now send only modem revision
                break;
            }
            default:
                Log.v(LOG_TAG,"Invalid request");
                break;

        }

        return returnObject;
    }

   /**
     * Response class representing ProvisionOEMTable and Revision
     *
     */
    static public class ProvisionTable {
        public int[] prv_tbl_oem = null;
        public int prv_tbl_rev = -1;

        public ProvisionTable (ByteBuffer buf) {
            Log.d(LOG_TAG, "ProvsionTableInfo: " + buf.toString());
            // Parse through the TLVs
            while (buf.hasRemaining() && buf.remaining() >= 3) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_GET_PROVISION_TABLE_OPTIONAL_TAG1:
                        byte[] data = new byte[length];
                        for (int i = 0; i < length; i++) {
                            data[i] = buf.get();
                        }
                        ByteBuffer wrapped = ByteBuffer.wrap(data);
                        wrapped.order(ByteOrder.LITTLE_ENDIAN);
                        this.prv_tbl_rev = wrapped.getInt();
                        Log.i(LOG_TAG, "Provision Table Rev = " + this.prv_tbl_rev);
                        break;

                    case TLV_TYPE_GET_PROVISION_TABLE_OPTIONAL_TAG2:
                        byte prv_tbl_oem_len = buf.get();
                        this.prv_tbl_oem = new int[prv_tbl_oem_len];
                        for (int i = 0; i < prv_tbl_oem_len; i++) {
                            this.prv_tbl_oem[i] = buf.getShort();
                        }
                        Log.i(LOG_TAG, "Provsions Table OEM = " +
                            Arrays.toString(this.prv_tbl_oem));
                        break;
                    default:
                        Log.i(LOG_TAG, "Invalid TLV type");
                        break;
                }
            }
        }
    }

    /**
     * Utility to convert an int array to a QmiArray of QmiInteger
     */
    private QmiArray<QmiInteger> intArrayToQmiArray (int[] arr){
        QmiInteger[] qmiIntArray = new QmiInteger[arr.length];
        for(int i = 0; i < arr.length; i++) {
            qmiIntArray[i] = new QmiInteger(arr[i]);
        }
        return new QmiArray<QmiInteger>(qmiIntArray, (short)arr.length, QmiInteger.class);
    }

    /**
     * A common representation of a request that only has the scenario number
     */
    public class ScenarioRequest extends BaseQmiStructType {
        public QmiArray<QmiInteger> list;

        public ScenarioRequest(int[] list) {
            this.list = intArrayToQmiArray(list);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {list};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_SCENARIO_ID};
        }

    }
}

