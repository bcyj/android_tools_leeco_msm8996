/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.util.Log;
import java.util.List;

import org.codeaurora.ims.csvt.CallForwardInfoP;
import org.codeaurora.ims.csvt.ICsvtService;
import org.codeaurora.ims.csvt.ICsvtServiceListener;
import com.qualcomm.ims.csvt.CsvtConstants;

public class VTPhoneConnection implements IVTConnection {
    private ICsvtService mVTInterface = null;
    public static final String VIDEO_CALL_SERVICE = "videophone";
    private Context mCtx;
    Handler mHandler;
    Handler mVideoCallWaitingHandler;
    Handler mVideoCallForwardingHandler;

    private long mConnectInfoStatus = 0;
    private boolean mHasPendingResultMessage = false;
    // XXX: VIDEOCALL_RESULT_REJECTCALL_EXCEPTION does not exist
    private int mPendingResultMessage;

    static private class CONNECT_INFO {
        static final long FALL_BACK_47 = (1L << 0) | (1L << 4);
        static final long FALL_BACK_57 = (1L << 1) | (1L << 4);
        static final long FALL_BACK_58 = (1L << 2) | (1L << 4);
        static final long FALL_BACK_88 = (1L << 3) | (1L << 4);
        static final long FALL_BACK = (1L << 4);
        // TODO: to be extended
    };

    private static final String TAG = "VT/VTPhoneConnection";

    private ICsvtServiceListener mVTListener = new ICsvtServiceListener.Stub() {
        public void onPhoneStateChanged(int state) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "VTConnection IVideoTelephonyListener.onVTStateChanged: " + state);
            if (mHandler == null)
                return;
            switch (state) {
            case CsvtConstants.CALL_STATE_IDLE:
                mHandler.sendMessage(Message.obtain(mHandler, IVTConnection.VTCALL_STATE_IDLE));
                break;
            case CsvtConstants.CALL_STATE_ACTIVE:
                mHandler.sendMessage(Message.obtain(mHandler, IVTConnection.VTCALL_STATE_ACTIVE));
                break;
            case CsvtConstants.CALL_STATE_RINGING:
                mHandler.sendMessage(Message.obtain(mHandler, IVTConnection.VTCALL_STATE_RINGING));
                break;
            case CsvtConstants.CALL_STATE_OFFHOOK:
                mHandler.sendMessage(Message.obtain(mHandler, IVTConnection.VTCALL_STATE_OFFHOOK));
                break;
            default:
                break;
            }
        }

        public void onCallStatus(int result) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "VTConnection IVideoTelephonyListener.onVTCallResultChanged: "
                        + result);
            if (mHandler == null) {
                mHasPendingResultMessage = true;
                mPendingResultMessage = result;
                return;
            } else {
                mHasPendingResultMessage = false;
            }

            // support call result:
            switch (result) {
            // new result info
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL_UNSPECIFIED:
            case CsvtConstants.CALL_STATUS_DISCONNECTED_ERROR_UNSPECIFIED:
                // call failed
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_NORMAL_UNSPECIFIED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_PROTOCOL_ERROR_UNSPECIFIED:
                // network busy
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_PROTOCOL_ERROR_UNSPECIFIED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NO_USER_RESPONDING:
                // no user responding
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_NO_USER_RESPONDING));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NETWORK_CONGESTION:
                // network congestion
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_NETWORK_CONGESTION));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_SERVICE_NOT_IMPLEMENTED:
                // bearer not supported
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_BEARER_NOT_SUPPORTED_65));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_SERVICE_OR_OPTION_NOT_IMPLEMENTED:
                // bearer not supported
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_BEARER_NOT_SUPPORTED_79));
                break;

            case CsvtConstants.CALL_STATUS_CONNECTED:
                mHandler.sendMessage(Message
                        .obtain(mHandler, IVTConnection.VTCALL_RESULT_CONNECTED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_LOST_SIGNAL:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_LOST_SIGNAL));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_INCOMING_MISSED:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_INCOMING_MISSED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NO_ANSWER:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_NO_ANSWER));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_BUSY:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_BUSY));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_INVALID_NUMBER:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_INVALID_NUMBER));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_INCOMING_REJECTED:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_INCOMING_REJECTED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_POWER_OFF:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_POWER_OFF));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_OUT_OF_SERVICE:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_OUT_OF_SERVICE));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_UNASSIGNED_NUMBER:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_UNASSIGNED_NUMBER));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NUMBER_CHANGED:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_NUMBER_CHANGED));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_LCOAL_PHONE_OUT_OF_3G_SERVICE:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED_LOCAL_OUT_OF_3G_SERVICE));
                break;

            // fall back related
            case CsvtConstants.CALL_STATUS_DISCONNECTED_RESOURCES_UNAVAILABLE: /*
                                                                                * fallback
                                                                                * #
                                                                                * 47
                                                                                */
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_FALLBACK_47));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_NOT_AUTHORIZATION: /*
                                                                                   * fallback
                                                                                   * #
                                                                                   * 57
                                                                                   */
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_FALLBACK_57));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_BEARER_NOT_AVAIL: /*
                                                                           * fallback
                                                                           * #58
                                                                           */
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_FALLBACK_58));
                break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_INCOMPATIBILITY_DESTINATION: /*
                                                                                      * fallback
                                                                                      * #
                                                                                      * 88
                                                                                      */
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_FALLBACK_88));
                break;

            // XXX: Exception related
            // case CsvtConstants.CALL_STATUS_CALL_EXCEPTION:
            // mHandler.sendMessage(Message.obtain( mHandler,
            // IVTConnection.VTCALL_RESULT_CALL_EXCEPTION));
            // break;

            // case CsvtConstants.CALL_STATUS_ANSWERCALL_EXCEPTION:
            // mHandler.sendMessage(Message.obtain( mHandler,
            // IVTConnection.VTCALL_RESULT_ANSWERCALL_EXCEPTION));
            // break;
            case CsvtConstants.CALL_STATUS_DISCONNECTED_NORMAL:
                mHandler.sendMessage(Message.obtain(mHandler,
                        IVTConnection.VTCALL_RESULT_DISCONNECTED));
                break;
            // XXX
            // case CsvtConstants.CALL_STATUS_ENDCALL_EXCEPTION:
            // case CsvtConstants.CALL_STATUS_FALLBACK_EXCEPTION:
            // case CsvtConstants.CALL_STATUS_REJECTCALL_EXCEPTION:
            default:
                break;
            }
        }

        public void onCallWaiting(boolean enabled) {
            Log.d(TAG, " oncall waiting");
            mVideoCallWaitingHandler.sendMessage(Message.obtain(mVideoCallWaitingHandler,
                    IVTConnection.MESSAGE_GET_CALL_WAITING, enabled));
        }

        /**
         * Called to notify about call forwarding options.
         *
         * @param fi
         *            , Call Forwarding options.
         */
        public void onCallForwardingOptions(List<CallForwardInfoP> fi) {
            mVideoCallForwardingHandler.sendMessage(Message.obtain(mVideoCallForwardingHandler,
                    IVTConnection.MESSAGE_GET_CF, fi));
        }

        public void onRingbackTone(boolean playTone) {
            mHandler.sendMessage(Message.obtain(mHandler,
                    IVTConnection.MESSAGE_RINGBACK_TONE, playTone));
        }
    };

    VTPhoneConnection(Context ctx) {
        // TODO:
        Log.e(TAG, "VTPhoneConnection abandoned constructor");
    }

    VTPhoneConnection(Context ctx, ICsvtService csvtService) {
        // TODO:
        mCtx = ctx;
        // mVTInterface =
        // ICsvtService.Stub.asInterface(ServiceManager.checkService(VIDEO_CALL_SERVICE));
        mVTInterface = csvtService;

        Log.e(TAG, "in VTPhoneConnect mVTInterface=" + mVTInterface);
        mHasPendingResultMessage = false;

        try {
            if(mVTInterface != null) {
                mVTInterface.registerListener(mVTListener);
            }else {
                Log.e(TAG, "not bind CsvtService.");
                return;
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while register listener or calling." + e.getMessage());
            return;
        }
    }

    public void clear() {
        try {
            mHasPendingResultMessage = false;
            mVTInterface.unregisterListener(mVTListener);
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while unregister listener or calling." + e.getMessage());
            return;
        }
    }

    public void acceptCall() {
        try {
            mVTInterface.acceptCall();
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while acceptCall." + e.getMessage());
        }
    }

    public void endSession() {
        try {
            mVTInterface.hangup();
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while endCall." + e.getMessage());
        }
    }

    public void rejectSession() {
        try {
            mVTInterface.rejectCall();
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while rejectCall." + e.getMessage());
        }
    }

    public void fallBack() {
        try {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "VTPhoneConnection.fallback...");
            mVTInterface.fallBack();
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while fallBack." + e.getMessage());
        }
    }

    public void setHandler(Handler h) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setHandler");
        mHandler = h;

        if (mHasPendingResultMessage && null != mVTListener) {
            try {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Handle pending result message!");
                mVTListener.onCallStatus(mPendingResultMessage);
                mHasPendingResultMessage = false;
            } catch (Exception e) {
                mHasPendingResultMessage = false;
                Log.e(TAG, "Exception occurs while setHandler." + e.getMessage());
            }
        }
    }

    public void setVideoCallWaitingHandler(Handler h) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setHandler");
        mVideoCallWaitingHandler = h;
    }

    public void setVideoCallForwardingHandler(Handler h) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setHandler");
        mVideoCallForwardingHandler = h;
    }

    // !!!NOTE: special function not inherit from IVTConnection
    public boolean call(String strNumber) {

        try {
            mVTInterface.dial(strNumber);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while call." + e.getMessage());
        }

        return false;
    }

    public void getCallForwardingOption(int commandInterfaceCFReason, Message onComplete) {

        try {
            mVTInterface.getCallForwardingOption(commandInterfaceCFReason, onComplete);
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while getCallForwardingOption." + e.getMessage());
        }
    }

    public void setCallForwardingOption(int commandInterfaceCFReason, int commandInterfaceCFAction,
            String dialingNumber, int timerSeconds, Message onComplete) {

        try {
            mVTInterface.setCallForwardingOption(commandInterfaceCFReason,
                    commandInterfaceCFAction, dialingNumber, timerSeconds, onComplete);
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while setCallForwardingOption." + e.getMessage());
        }
    }

    public void getCallWaiting(Message onComplete) {

        try {
            mVTInterface.getCallWaiting(onComplete);
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while getCallWaiting." + e.getMessage());
        }
    }

    public void setCallWaiting(boolean enable, Message onComplete) {

        try {
            mVTInterface.setCallWaiting(enable, onComplete);
        } catch (Exception e) {
            Log.e(TAG, "Exception occurs while setCallWaiting." + e.getMessage());
        }
    }

}
