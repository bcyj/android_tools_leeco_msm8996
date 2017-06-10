/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiStructType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiByte;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiInteger;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiNull;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiString;


public class PresenceOemHook  {
    private static String LOG_TAG = "PresenceOemHook";

    private static final short PRESENCE_SERVICE_ID = 0x03; // This value is from IDL

    public static final int OEM_HOOK_UNSOL_IND = 1;


    public static final short QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER = 0x0020;
    public static final short QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE = 0x0021;
    public static final short QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE = 0x0022;
    public static final short QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE = 0x0023;

    public static final short QCRILHOOK_PRESENCE_IMS_ENABLER_STATE_REQ = 0x0024;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_REQ  = 0x0025;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_XML_REQ = 0x0026;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_UNPUBLISH_REQ = 0x0027;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_REQ= 0x0028;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_XML_REQ = 0x0029;
    public static final short QCRILHOOK_PRESENCE_IMS_SEND_UNSUBSCRIBE_REQ = 0x002A;
    public static final short QCRILHOOK_PRESENCE_IMS_SET_NOTIFY_FMT_REQ = 0x002B;
    public static final short QCRILHOOK_PRESENCE_IMS_GET_NOTIFY_FMT_REQ = 0x002C;
    public static final short QCRILHOOK_PRESENCE_IMS_SET_EVENT_REPORT_REQ = 0x002D;
    public static final short QCRILHOOK_PRESENCE_IMS_GET_EVENT_REPORT_REQ = 0x002E;

    public final static String IMS_ENABLER_RESPONSE[] = {
        "UNKNOWN",
        "UNINITIALIZED",
        "INITIALIZED",
        "AIRPLANE",
        "REGISTERED"
    };

    public static enum SubscriptionType {
        NONE, SIMPLE, POLLING
    }


    /* PACKAGE SCOPE */
    Context mContext;

    /* PRIVATE SCOPE */

    private QmiOemHook mQmiOemHook;

    private static int mRefCount = 0;

    private static PresenceOemHook mInstance;

    /* This private constructor associates the listenerLooper with
     * OEM HOOK library's broadcast receiver. So the message would
     * be received by main thread during execution of broadcast
     * receiver and then it would be delegated to client's listener
     * thread for background parsing and any other heavy work, later
     * this background thread may choose to inform main thread for
     * any UI update.
     */
    private PresenceOemHook(Context context, Looper listenerLooper) {
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
    public static PresenceOemHook getInstance(Context context, Handler listenerHandler) {
        if (mInstance == null) {
            mInstance = new PresenceOemHook(context, listenerHandler.getLooper());
            /*
             * This registers PresenceOemHook with QmiOemHook library so that it can
             * receive any Presence specific responses/unsols
             */

            QmiOemHook.registerService(PRESENCE_SERVICE_ID,
                    listenerHandler, OEM_HOOK_UNSOL_IND);


            Log.v(LOG_TAG, "Registered PresenceOemHook with QmiOemHook");
        } else {
            mInstance.mContext = context;
        }
        mRefCount++;
        return mInstance;
    }

    public synchronized void dispose() {
        mRefCount--;
        if (mRefCount == 0) {
            Log.v(LOG_TAG, "dispose(): Unregistering service");
            QmiOemHook.unregisterService(PRESENCE_SERVICE_ID);
            mQmiOemHook.dispose();
            mInstance = null;

        } else {
            Log.v(LOG_TAG, "dispose mRefCount = " + mRefCount);
        }
    }

    public Object imsp_get_enabler_state_req() {

        PresenceMsgBuilder.NoTlvPayloadRequest req = new PresenceMsgBuilder
            .NoTlvPayloadRequest();
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_ENABLER_STATE_REQ,
                    req.getTypes(),
                    req.getItems());
            return (Object) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Integer imsp_send_publish_req(int publish_status,
            String contact_uri,
            String description,
            String ver,
            String service_id,
            int is_audio_supported,
            int audio_capability,
            int is_video_supported,
            int video_capability) {

        PresenceMsgBuilder.Publish.PublishStructRequest req = new PresenceMsgBuilder
            .Publish.PublishStructRequest(publish_status,
                contact_uri,
                description,
                ver,
                service_id,
                is_audio_supported,
                audio_capability,
                is_video_supported,
                video_capability);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_REQ, req.getTypes(),
                    req.getItems());
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Integer imsp_send_publish_xml_req(String xml) {
        PresenceMsgBuilder.Publish.PublishXMLRequest req = new PresenceMsgBuilder
            .Publish.PublishXMLRequest(xml);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_XML_REQ, req.getTypes(),
                    req.getItems());
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }


    public Integer imsp_send_unpublish_req() {

        PresenceMsgBuilder.UnPublish.UnPublishRequest req = new PresenceMsgBuilder
            .UnPublish.UnPublishRequest();
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_UNPUBLISH_REQ, null, null);
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }


    public Integer imsp_send_subscribe_req(SubscriptionType subscriptionType,
            ArrayList<String> contactList) {

        PresenceMsgBuilder.Subscribe.SubscribeStructRequest req = new PresenceMsgBuilder
            .Subscribe.SubscribeStructRequest(subscriptionType, contactList);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_REQ,
                    req.getTypes(),
                    req.getItems());
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Integer imsp_send_subscribe_xml_req(String xml) {

        PresenceMsgBuilder.Subscribe.SubscribeXMLRequest req = new PresenceMsgBuilder
            .Subscribe.SubscribeXMLRequest(xml);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_XML_REQ,
                    req.getTypes(),
                    req.getItems());
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }


    public Integer imsp_send_unsubscribe_req(String peerURI) {

        PresenceMsgBuilder.UnSubscribe.UnSubscribeRequest req = new PresenceMsgBuilder
            .UnSubscribe.UnSubscribeRequest(peerURI);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SEND_UNSUBSCRIBE_REQ,
                    req.getTypes(),
                    req.getItems());
            return (Integer) receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Object imsp_set_notify_fmt_req(int flag) {

        PresenceMsgBuilder.NotifyFmt.SetFmt req = new PresenceMsgBuilder
            .NotifyFmt.SetFmt((short)flag);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SET_NOTIFY_FMT_REQ,
                    req.getTypes(),
                    req.getItems());
            return  receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Object imsp_get_notify_fmt_req() {

        PresenceMsgBuilder.NoTlvPayloadRequest req = new PresenceMsgBuilder
            .NoTlvPayloadRequest();
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_GET_NOTIFY_FMT_REQ, null, null);
            return  receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Object imsp_set_event_report_req(int mask) {

        PresenceMsgBuilder.EventReport.SetEventReport req = new PresenceMsgBuilder
            .EventReport.SetEventReport(mask);
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_SET_EVENT_REPORT_REQ,
                    req.getTypes(),
                    req.getItems());
            return  receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Object imsp_get_event_report_req() {

        PresenceMsgBuilder.NoTlvPayloadRequest req = new PresenceMsgBuilder
            .NoTlvPayloadRequest();
        try {
            HashMap<Integer, Object> hashMap = mQmiOemHook.sendQmiMessageSync(
                    PRESENCE_SERVICE_ID,
                    QCRILHOOK_PRESENCE_IMS_GET_EVENT_REPORT_REQ, null, null);
            return  receive(hashMap);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    static public class PresenceUnsolIndication {
        public int oemHookMesgId;
        public Object obj;
    }

    static public class PresenceSolResponse {
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
            case QCRILHOOK_PRESENCE_IMS_ENABLER_STATE_REQ:
            {
                int enablerState = 0;
                PresenceSolResponse presenceSolResp = new PresenceSolResponse();

                if (successStatus == 0) {

                    enablerState = PresenceMsgParser.parseEnablerState(respByteBuf);

                    Log.v(LOG_TAG, "Enabler state = " + enablerState);

                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = enablerState;
                    returnObject = presenceSolResp;

                    String state = IMS_ENABLER_RESPONSE[enablerState];

                    Log.v(LOG_TAG,
                            "Response: QCRILHOOK_PRESENCE_IMS_ENABLER_STATE_REQ="
                            + state);
                    break;
                } else {
                    Log.v(LOG_TAG,
                            "OemHookError: QCRILHOOK_PRESENCE_IMS_ENABLER_STATE_REQ="
                            + successStatus);

                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = enablerState;

                    return presenceSolResp; // enabler state = UNKNOWN
                }

            }
            case QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_REQ:
            {
                Log.v(LOG_TAG, "Response: QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_REQ="
                        + successStatus);

                break;
            }
            case QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_XML_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SEND_PUBLISH_XML_REQ="
                        + successStatus);

                break;
            }
            case QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_REQ="
                        + successStatus);

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_XML_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SEND_SUBSCRIBE_XML_REQ="
                        + successStatus);

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_SEND_UNPUBLISH_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SEND_UNPUBLISH_REQ="
                        + successStatus);

                break;
            }
            case QCRILHOOK_PRESENCE_IMS_SEND_UNSUBSCRIBE_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SEND_UNSUBSCRIBE_REQ="
                        + successStatus);

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE:
            {
                Log.v(LOG_TAG, "Ind: QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE="
                        + successStatus);
                PresenceUnsolIndication presenceUnSolInd = new PresenceUnsolIndication();
                presenceUnSolInd.oemHookMesgId = QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_UPDATE;
                presenceUnSolInd.obj = PresenceMsgParser
                    .parseNotifyUpdate(respByteBuf,
                            responseSize,
                            successStatus);

                returnObject = presenceUnSolInd;

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE:
            {
                Log.v(LOG_TAG,
                        "Ind: QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE="
                        + successStatus);

                if (false) {

                    // UT only. ...start
                    for (int i = 0; i < 7; i++) {
                        respByteBuf.get();
                    }
                    // UT only. ...End
                }

                String xml = PresenceMsgParser.parseNotifyUpdateXML(respByteBuf);

                PresenceUnsolIndication presenceUnSolInd = new PresenceUnsolIndication();
                presenceUnSolInd.oemHookMesgId = QCRILHOOK_PRESENCE_IMS_UNSOL_NOTIFY_XML_UPDATE;
                presenceUnSolInd.obj = xml;

                returnObject = presenceUnSolInd;

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_SET_NOTIFY_FMT_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SET_NOTIFY_FMT_REQ="
                        + successStatus);

                PresenceSolResponse presenceSolResp = new PresenceSolResponse();
                presenceSolResp.result = successStatus;
                presenceSolResp.data = -1;

                returnObject = presenceSolResp;
                break;
            }

            case QCRILHOOK_PRESENCE_IMS_GET_NOTIFY_FMT_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_GET_NOTIFY_FMT_REQ="
                        + successStatus);

                PresenceSolResponse presenceSolResp = new PresenceSolResponse();

                if (successStatus == 0) {

                    int val = PresenceMsgParser.parseGetNotifyReq(respByteBuf);

                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = val;

                    Log.v(LOG_TAG,
                            "Response: QCRILHOOK_PRESENCE_IMS_GET_NOTIFY_FMT_REQ"
                            + " update_with_struct_info=" + val);

                } else {
                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = -1;

                }
                returnObject = presenceSolResp;

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_SET_EVENT_REPORT_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_SET_EVENT_REPORT_REQ="
                        + successStatus);

                PresenceSolResponse presenceSolResp = new PresenceSolResponse();
                presenceSolResp.result = successStatus;
                presenceSolResp.data = -1;

                returnObject = presenceSolResp;

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_GET_EVENT_REPORT_REQ:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_GET_EVENT_REPORT_REQ="
                        + successStatus);

                PresenceSolResponse presenceSolResp = new PresenceSolResponse();

                if (successStatus == 0) {

                    int val = PresenceMsgParser.parseGetEventReport(respByteBuf);

                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = val;

                    Log.v(LOG_TAG,
                            "Response: QCRILHOOK_PRESENCE_IMS_GET_EVENT_REPORT_REQ"
                            + " event_report_bit_masks=" + val);


                } else {
                    presenceSolResp.result = successStatus;
                    presenceSolResp.data = -1;

                }
                returnObject = presenceSolResp;

                break;
            }

            case QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER="
                        + successStatus);

                int val = PresenceMsgParser.parsePublishTrigger(respByteBuf);

                PresenceUnsolIndication ind = new PresenceUnsolIndication();
                ind.oemHookMesgId = QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER;
                ind.obj = val;

                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_UNSOL_PUBLISH_TRIGGER result="
                        + successStatus
                        + " publish_trigger=" + val);

                returnObject = ind;
                break;
            }

            case QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE:
            {
                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE="
                        + successStatus);


                int val = PresenceMsgParser.parseEnablerStateInd(respByteBuf);

                PresenceUnsolIndication ind = new PresenceUnsolIndication();
                ind.oemHookMesgId = QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE;
                ind.obj = val;

                Log.v(LOG_TAG,
                        "Response: QCRILHOOK_PRESENCE_IMS_UNSOL_ENABLER_STATE result="
                        + successStatus
                        + " enabler_state=" + val);

                returnObject = ind;
                break;
            }

        }

        return returnObject;
    }

    /**
     * Handles UNSOL and Async Responses,
     * called from listener thread.
     */
    public static Object handleMessage(Message msg) {
        switch (msg.what) {
            case OEM_HOOK_UNSOL_IND:
                AsyncResult ar = (AsyncResult) msg.obj;
                HashMap<Integer, Object> map = (HashMap<Integer, Object>) ar.result;
                if (map == null) {
                    Log.e(LOG_TAG, "Hashmap async userobj is NULL");
                    return null;
                }
                return PresenceOemHook.receive(map);
            default:
                Log.d(LOG_TAG,"Recieved msg.what="+msg.what);
        }
        return null;
    }

    protected void finalize() {
        Log.v(LOG_TAG, "finalize() hit");
    }
}
