/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.lanixcarrierpack;

import android.content.Context;
import android.os.AsyncResult;
import android.util.Log;

import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class QcRilHookInterface {
    public interface QcrilHookInterfaceListener {
        public void onQcRilHookReady();
    }

    private static final String LOG_TAG = "QcrilHookInterface";
    private static final boolean DBG = true;//Log.isLoggable(LOG_TAG, Log.DEBUG);
    // personalization type
    private final int GWNW_PERSONALIZATION = 3;

    private final static int INT_SIZE = 4;
    private final static int BYTE_SIZE = 1;

    private Context mContext;
    private QcRilHook mQcrilHook;
    private QcrilHookInterfaceListener mListener;
    private boolean serviceConnected;

    /**
     * Called when connection to QcrilMsgTunnelService has been established.
     */
    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            serviceConnected = true;
            mListener.onQcRilHookReady();
        }
    };

    public QcRilHookInterface(Context context,
            QcrilHookInterfaceListener listener) {
        mContext = context;
        this.mListener = listener;
        mQcrilHook = new QcRilHook(context, mQcrilHookCb);
    }

    public int setGWNWPersonalization(String pin) {
        if (pin != null && !pin.isEmpty()) {
            pin += '\0'; //null terminate string
            int payloadLength = pin.getBytes().length + BYTE_SIZE;
            byte[] payload = new byte[payloadLength];
            log("pin sent to socket = " + pin);
            ByteBuffer buf = ByteBuffer.wrap(payload);
            buf.order(ByteOrder.nativeOrder());

            buf.put(pin.getBytes());
            buf.put((byte) GWNW_PERSONALIZATION);

            log("sending buff :"+buf+" payload:"+payload.length);
            AsyncResult ar = mQcrilHook.sendQcRilHookMsg(IQcRilHook.QCRIL_EVT_HOOK_SET_PERSONALIZATION
                    , payload);
            if (ar.exception != null) {
                log("QCRIL_EVT_HOOK_SET_PERSONALIZATION failed w/ " + ar.exception
                        + " ar.result:"+ar.result);
                if (ar.result != null) {
                    byte[] result = (byte[]) ar.result;
                    ByteBuffer byteBuf = ByteBuffer.wrap(result);
                    byteBuf.order(ByteOrder.nativeOrder());
                    log("perso response buffer: " + byteBuf.toString());
                    int retry = byteBuf.getInt();
                    log("perso response retry: " + retry);
                    if (retry >= 0) {
                        return retry;
                    }
                }
                return -1;
            }
            if (ar.result == null) {
                log("QCRIL_EVT_HOOK_SET_PERSONALIZATION failed w/ null result");
                return -1;
            } else {
                return Integer.MAX_VALUE;//operation is successful.
            }
        } else {
            log("set with incorrect params: " + pin + "persotype: " + GWNW_PERSONALIZATION);
            return -1;
        }
    }

    public boolean isDeviceNetworkPersonalized() {
        AsyncResult ar = mQcrilHook
                .sendQcRilHookMsg(IQcRilHook.QCRIL_EVT_HOOK_GET_PERSONALIZATION_STATUS);

        if (ar.exception != null) {
            log("QCRIL_EVT_HOOK_GET_PERSONALIZATION_STATUS failed w/ " + ar.exception);
            return false;
        }

        if (ar.result == null) {
            log("QCRIL_EVT_HOOK_GET_PERSONALIZATION_STATUS failed w/ null result");
            return false;
        } else {
            byte[] resultArray = (byte[]) ar.result;
            ByteBuffer byteBuf = ByteBuffer.wrap(resultArray);
            int hasNWPerso = byteBuf.get();
            log("received buffer: " + byteBuf.toString());
            log("has network perso param:" + hasNWPerso);
            if (hasNWPerso > 0) {
                int personalized = byteBuf.getInt();
                log("QCRIL_EVT_HOOK_GET_PERSONALIZATION_STATUS:" + personalized);
                return personalized >= 0;
            }
            return false;
        }
    }

    public synchronized void dispose() {
        log("dispose(): Unregistering QcRilHook and calling QcRilHook dispose");
        mQcrilHook.dispose();
    }

    private void log(String msg) {
        if (DBG) {
            Log.d(LOG_TAG, msg);
        }
    }
}
