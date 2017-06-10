/******************************************************************************
 * @file    EmbmsOemHook.java
 *
 * This is compatible with version 00.08.01 of the IDL.
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcrilhook;


import java.io.IOException;
import java.lang.StringBuilder;
import java.nio.ByteBuffer;
import java.nio.BufferUnderflowException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

import android.content.Context;
import android.os.Handler;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.util.Log;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiStructType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.*;


public class EmbmsOemHook extends Handler{
    private static String LOG_TAG = "EmbmsOemHook";

    private static final int SIZE_OF_TMGI = 6;
    private static final int SUCCESS = 0;
    private static final int FAILURE = -1;
    private static final int DEFAULT_PHONE = 0;
    /*
     * size in bytes of the length within V field of TLV.
     *
     * In a TLV, if V is an Array, it is packed as follows:
     *   N+Array, N is the number of elements. N could be
     *   represented as a byte or 2 bytes, depending on whether
     *   the maximum size of the array is <=255 or not.
     */
    private static final short ONE_BYTE = 1;
    private static final short TWO_BYTES = 2;

    // Identifies the response as an OEM HOOK response
    private static final int OEM_HOOK_RESPONSE = 1;
    //Identifies the call back from QCRilHook when QcrilMsgtunnelService is ready
    private static final int QCRILHOOK_READY_CALLBACK = 2;

    // The following private constants are taken from the IDL qmi_embms_v01.idl
    // defining the interface between Android Telephony and QMI RIL
    private static final short EMBMS_SERVICE_ID = 0x02;

    //The base/offset for QcRilHook's specific unsol
    //This offset should be used to avoid any overlap with UNSOL from RIL's IDL
    private static final int UNSOL_BASE_QCRILHOOK = 0x1000;


    /**
     * Values for the TYPE field of TLVs accompanying a
     * message/indication
     */

    // The Debug trace ID will be the first mandatory parameter in the request
    // and the first in the response.
    // The response code will be the second mandatory parameter in the response.
    // So, the value for the TYPE field will be the same for all requests/responses.
    // Defining a generic constant for that
    private static final byte TLV_TYPE_COMMON_REQ_TRACE_ID = 0x01;
    private static final byte TLV_TYPE_COMMON_RESP_TRACE_ID = 0x01;
    private static final byte TLV_TYPE_COMMON_RESP_CODE = 0x02;

    // For all the requests/responses that use call id, the position of the parameter
    // in the list is the same.
    // Defining a generic constant for that too
    private static final byte TLV_TYPE_COMMON_REQ_CALL_ID = 0x02;
    private static final byte TLV_TYPE_COMMON_RESP_CALL_ID = 0x10;

    //enable
    private static final byte TLV_TYPE_ENABLE_RESP_IFNAME = 0x11;
    private static final byte TLV_TYPE_ENABLE_RESP_IF_INDEX = 0x12;
    //activate
    private static final byte TLV_TYPE_ACTIVATE_REQ_TMGI = 0x03;
    private static final byte TLV_TYPE_ACTIVATE_REQ_PRIORITY = 0x04;
    private static final byte TLV_TYPE_ACTIVATE_REQ_EARFCN_LIST = 0x05;
    private static final byte TLV_TYPE_ACTIVATE_REQ_SAI_LIST = 0x10;
    private static final byte TLV_TYPE_ACTIVATE_RESP_TMGI = 0x11;
    //deactivate
    private static final byte TLV_TYPE_DEACTIVATE_REQ_TMGI = 0x03;
    private static final byte TLV_TYPE_DEACTIVATE_RESP_TMGI = 0x11;
    //activate
    private static final byte TLV_TYPE_ACTDEACTIVATE_REQ_ACT_TMGI = 0x03;
    private static final byte TLV_TYPE_ACTDEACTIVATE_REQ_DEACT_TMGI = 0x04;
    private static final byte TLV_TYPE_ACTDEACTIVATE_REQ_PRIORITY = 0x05;
    private static final byte TLV_TYPE_ACTDEACTIVATE_REQ_EARFCN_LIST = 0x06;
    private static final byte TLV_TYPE_ACTDEACTIVATE_REQ_SAI_LIST = 0x10;
    private static final byte TLV_TYPE_ACTDEACTIVATE_RESP_ACT_CODE = 0x02;
    private static final byte TLV_TYPE_ACTDEACTIVATE_RESP_DEACT_CODE = 0x03;
    private static final byte TLV_TYPE_ACTDEACTIVATE_RESP_ACTTMGI = 0x11;
    private static final byte TLV_TYPE_ACTDEACTIVATE_RESP_DEACTTMGI = 0x12;
    //getAvailableTMGIList
    private static final byte TLV_TYPE_GET_AVAILABLE_RESP_TMGI_ARRAY = 0x10;
    //getActiveTMGIList
    private static final byte TLV_TYPE_GET_ACTIVE_RESP_TMGI_ARRAY = 0x10;
    //getCoverageState
    private static final byte TLV_TYPE_GET_COVERAGE_STATE_RESP_STATE = 0x10;
    //getSignalStrength
    private static final byte TLV_TYPE_GET_SIG_STRENGTH_RESP_MBSFN_AREA_ID = 0x10;
    private static final byte TLV_TYPE_GET_SIG_STRENGTH_RESP_SNR = 0x11;
    private static final byte TLV_TYPE_GET_SIG_STRENGTH_RESP_EXCESS_SNR = 0x12 ;
    private static final byte TLV_TYPE_GET_SIG_STRENGTH_RESP_NUMBER_OF_TMGI_PER_MBSFN = 0x13 ;
    private static final byte TLV_TYPE_GET_SIG_STRENGTH_RESP_ACTIVE_TMGI_LIST = 0x14;
    // getTime
    private static final byte TLV_TYPE_GET_TIME_RESP_TIME_MSECONDS = 0x03;
    private static final byte TLV_TYPE_GET_TIME_RESP_DAY_LIGHT_SAVING = 0x10;
    private static final byte TLV_TYPE_GET_TIME_RESP_LEAP_SECONDS = 0x11;
    private static final byte TLV_TYPE_GET_TIME_RESP_LOCAL_TIME_OFFSET = 0x12;
    // setTime
    private static final byte TLV_TYPE_SET_TIME_REQ_SNTP_SUCCESS = 0x01;
    private static final byte TLV_TYPE_SET_TIME_REQ_TIME_MSECONDS = 0x10;
    private static final byte TLV_TYPE_SET_TIME_REQ_TIME_STAMP = 0x11;
    //getActiveLogPacketIDs
    private static final short TLV_TYPE_ACTIVELOGPACKETID_REQ_PACKET_ID_LIST = 0x02;
    private static final short TLV_TYPE_ACTIVELOGPACKETID_RESP_PACKET_ID_LIST = 0x02;
    //deliverLogPacket
    private static final short TLV_TYPE_DELIVERLOGPACKET_REQ_PACKET_ID = 0x02;
    private static final short TLV_TYPE_DELIVERLOGPACKET_REQ_LOG_PACKET = 0x03;
    //getE911State
    private static final short TLV_TYPE_GET_E911_RESP_STATE = 0x10;
    //contentDescription
    private static final byte TLV_TYPE_CONTENT_DESCRIPTION_REQ_TMGI = 0x03;
    private static final byte TLV_TYPE_CONTENT_DESCRIPTION_REQ_PARAMETER_ARRAY = 0x10;
    //getPLMNList
    //TO DO: Add the correct TLV
    private static final byte TLV_TYPE_GET_PLMN_LIST_RESP_PLMN_LIST = 0x10;
    //getEmbmsStatus
    private static final short TLV_TYPE_GET_EMBMS_STATUS_RESP = 0x02;

    //unsol service state indication
    private static final short TLV_TYPE_UNSOL_STATE_IND_STATE = 0x01;
    private static final short TLV_TYPE_UNSOL_STATE_IND_IP_ADDRESS = 0x02;
    private static final short TLV_TYPE_UNSOL_STATE_IND_IF_INDEX = 0x03;
    //unsol active TMGI list indication
    private static final short TLV_TYPE_UNSOL_ACTIVE_IND_TMGI_ARRAY = 0x02;
    //unsol broadcast coverage state indication
    private static final short TLV_TYPE_UNSOL_COVERAGE_IND_STATE_OR_RESPONSE_CODE = 0x02;
    //unsol available TMGI list indication
    private static final short TLV_TYPE_UNSOL_AVAILABLE_IND_TMGI_ARRAY_OR_RESPONSE_CODE = 0x02;
    //unsol OOS state indication
    private static final short TLV_TYPE_UNSOL_OOS_IND_STATE = 0x02;
    private static final short TLV_TYPE_UNSOL_OOS_IND_TMGI_ARRAY = 0x03;
    //unsol cell ID indication
    private static final short TLV_TYPE_UNSOL_CELL_ID_IND_MCC = 0x02;
    private static final short TLV_TYPE_UNSOL_CELL_ID_IND_MNC = 0x03;
    private static final short TLV_TYPE_UNSOL_CELL_ID_IND_CID = 0x04;
    //unsol Radio state indication
    private static final short TLV_TYPE_UNSOL_RADIO_STATE = 0x02;
    //unsol SAI Notification
    private static final short TLV_TYPE_UNSOL_SAI_IND_CAMPED_SAI_LIST = 0x02;
    private static final short TLV_TYPE_UNSOL_SAI_IND_SAI_PER_GROUP_LIST = 0x03;
    private static final short TLV_TYPE_UNSOL_SAI_IND_AVAILABLE_SAI_LIST = 0x04;
    // unsol SIB16 coverage notification
    private static final short TLV_TYPE_UNSOL_SIB16 = 0x01;
    // unsol E911 state notification
    private static final short TLV_TYPE_UNSOL_E911_STATE_OR_RESPONSE_CODE = 0x02;
    //unsol contentDescriptionPerObjectControl
    private static final short TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_TMGI = 0x02;
    private static final short TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_CONTENT_CONTROL = 0x10;
    private static final short TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_STATUS_CONTROL = 0x11;
    // unsol EMBMS status
    private static final short TLV_TYPE_UNSOL_EMBMS_STATUS = 0x01;

    //Message IDs for the requests and the unsols
    private static final short EMBMSHOOK_MSG_ID_ENABLE = 0x0;
    private static final short EMBMSHOOK_MSG_ID_DISABLE = 0x1;
    private static final short EMBMSHOOK_MSG_ID_ACTIVATE = 0x2;
    private static final short EMBMSHOOK_MSG_ID_DEACTIVATE = 0x3;
    private static final short EMBMSHOOK_MSG_ID_GET_AVAILABLE = 0x4;
    private static final short EMBMSHOOK_MSG_ID_GET_ACTIVE = 0x5;
    private static final short EMBMSHOOK_MSG_ID_GET_COVERAGE = 0x8;
    private static final short EMBMSHOOK_MSG_ID_GET_SIG_STRENGTH = 0x9;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_STATE_CHANGE = 0x0B;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_ACTIVE_TMGI_LIST = 0x0C;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_COVERAGE_STATE = 0x0D;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_AVAILABLE_TMGI_LIST = 0x0F;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_OOS_STATE = 0x10;
    private static final short EMBMSHOOK_MSG_ID_ACTDEACT = 0x11;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_CELL_ID = 0x12;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_RADIO_STATE = 0x13;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_SAI_LIST = 0x14;
    private static final short EMBMSHOOK_MSG_ID_GET_ACTIVE_LOG_PACKET_IDS = 0x15;
    private static final short EMBMSHOOK_MSG_ID_DELIVER_LOG_PACKET = 0x16;
    private static final short EMBMSHOOK_MSG_ID_SET_TIME = 0x17;
    private static final short EMBMSHOOK_MSG_ID_GET_SIB16_COVERAGE = 0x18;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_SIB16 = 0x19;
    private static final short EMBMSHOOK_MSG_ID_GET_TIME = 0x1A;
    private static final short EMBMSHOOK_MSG_ID_GET_E911_STATE = 0x1B;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_E911_STATE = 0x1C;
    private static final short EMBMSHOOK_MSG_ID_CONTENT_DESCRIPTION = 0x1D;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL = 0x1E;
    private static final short EMBMSHOOK_MSG_ID_GET_PLMN_LIST = 0x1F;
    private static final short EMBMSHOOK_MSG_ID_UNSOL_EMBMS_STATUS = 0x20;
    private static final short EMBMSHOOK_MSG_ID_GET_EMBMS_STATUS = 0x21;

    public static final int UNSOL_TYPE_STATE_CHANGE = 0x01;
    public static final int UNSOL_TYPE_ACTIVE_TMGI_LIST = 0x02;
    public static final int UNSOL_TYPE_BROADCAST_COVERAGE = 0x03;
    public static final int UNSOL_TYPE_AVAILABLE_TMGI_LIST = 0x04;
    public static final int UNSOL_TYPE_OOS_STATE = 0x05;
    public static final int UNSOL_TYPE_CELL_ID = 0x06;
    public static final int UNSOL_TYPE_RADIO_STATE = 0x07;
    public static final int UNSOL_TYPE_SAI_LIST = 0x08;
    public static final int UNSOL_TYPE_SIB16_COVERAGE = 0x09;
    public static final int UNSOL_TYPE_E911_STATE = 0x0A;
    public static final int UNSOL_TYPE_CONTENT_DESC_PER_OBJ_CONTROL = 0x0B;
    public static final int UNSOL_TYPE_EMBMS_STATUS = 0x0C;
    public static final int UNSOL_TYPE_EMBMSOEMHOOK_READY_CALLBACK = UNSOL_BASE_QCRILHOOK + 0x01;


    // Holds the singleton instance
    private static EmbmsOemHook sInstance;
    private static int mRefCount = 0;

    private QmiOemHook mQmiOemHook;
    private RegistrantList mRegistrants;

    // Suppress default constructor for non-instantiability
    private EmbmsOemHook (Context context) {
        Log.v(LOG_TAG, "EmbmsOemHook ()");
        mQmiOemHook = QmiOemHook.getInstance(context);
        QmiOemHook.registerService(EMBMS_SERVICE_ID, this,
                OEM_HOOK_RESPONSE);
        QmiOemHook.registerOnReadyCb(this, QCRILHOOK_READY_CALLBACK, null);
        mRegistrants = new RegistrantList();
    }

    // This class is a singleton
    public static synchronized EmbmsOemHook getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new EmbmsOemHook(context);
            Log.d(LOG_TAG, "Singleton Instance of Embms created.");
        }
        mRefCount++;
        return sInstance;
    }

    /**
     * Client is expected to call this when they no longer need to use this
     * object.
     * When the last client calls 'dispose', we clean up the singleton object
     */
    public synchronized void dispose() {
        if (--mRefCount == 0) {
            Log.d(LOG_TAG, "dispose(): Unregistering receiver");
            QmiOemHook.unregisterService(EMBMS_SERVICE_ID);
            QmiOemHook.unregisterOnReadyCb(this);
            mQmiOemHook.dispose();
            mQmiOemHook = null;
            sInstance = null;
            mRegistrants.removeCleared();
        } else {
            Log.v(LOG_TAG, "dispose mRefCount = " + mRefCount);
        }
    }

    /**
     * Handles UNSOL and Async Responses
     */
    @Override
    public void handleMessage(Message msg) {
        Log.i(LOG_TAG, "received message : " + msg.what);
        AsyncResult ar = (AsyncResult) msg.obj;

        switch (msg.what) {
            case OEM_HOOK_RESPONSE:
                HashMap<Integer, Object> map = (HashMap<Integer, Object>) ar.result;
                if (map == null) {
                    Log.e(LOG_TAG, "Hashmap async userobj is NULL");
                    return;
                }

                handleResponse(map);
                break;
            case QCRILHOOK_READY_CALLBACK:
                Object payload = ar.result;
                //There is only one ready callback, notify clients with default phone.
                notifyUnsol(UNSOL_TYPE_EMBMSOEMHOOK_READY_CALLBACK, payload, DEFAULT_PHONE);
                break;
            default:
                Log.e(LOG_TAG, "Unexpected message received from QmiOemHook what = "
                                + msg.what);
                break;
        }
    }

    /**
     * Parses all the responses - sync, async, unsol
     * The request ID + response_size + service_id + Message_id + status
     * have already been parsed and are stored individually in the map
     * when we receive them.
     * Based on the message ID we will parse the remainder of the buffer into
     * the appropriate response object.
     * @param map
     * @return Object
     */
    private void handleResponse(HashMap<Integer, Object> map) {

        short msgId = (Short) map.get(QmiOemHookConstants.MESSAGE_ID);
        int responseSize = (Integer) map.get(QmiOemHookConstants.RESPONSE_TLV_SIZE);
        int successStatus = (Integer) map.get(QmiOemHookConstants.SUCCESS_STATUS);
        Message msg = (Message) map.get(QmiOemHookConstants.MESSAGE);
        int phoneId = (Integer) map.get(QmiOemHookConstants.PHONE_ID);
        if (msg != null) {
            msg.arg1 = phoneId;
        }
        ByteBuffer respByteBuf = (ByteBuffer) map.get(QmiOemHookConstants.RESPONSE_BUFFER);
        AsyncResult ar;

        Log.d(LOG_TAG, " responseSize=" + responseSize + " successStatus=" + successStatus +
                "phoneId: " + phoneId);

        // Create the appropriate response object by parsing the buffer.
        switch (msgId) {
            case EMBMSHOOK_MSG_ID_ENABLE: {
                msg.obj = new EnableResponse(successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_DISABLE: {
                msg.obj = new DisableResponse(successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_DEACTIVATE:
            case EMBMSHOOK_MSG_ID_ACTIVATE: {
                msg.obj = new TmgiResponse (successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_ACTDEACT: {
                msg.obj = new ActDeactResponse (successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_STATE_CHANGE: {
                StateChangeInfo info = new StateChangeInfo(respByteBuf);
                notifyUnsol(UNSOL_TYPE_STATE_CHANGE, info, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_AVAILABLE_TMGI_LIST:
                //intentional fall through
            case EMBMSHOOK_MSG_ID_GET_AVAILABLE: {
                // The client does not have a callback for this response.
                // So send it as an unsol.

                if (msgId == EMBMSHOOK_MSG_ID_GET_AVAILABLE && successStatus != SUCCESS) {
                    Log.e(LOG_TAG, "Error received in EMBMSHOOK_MSG_ID_GET_AVAILABLE: "
                                    + successStatus);
                    break;
                }
                TmgiListIndication list = new TmgiListIndication(respByteBuf, msgId);
                notifyUnsol(UNSOL_TYPE_AVAILABLE_TMGI_LIST, list, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_ACTIVE_TMGI_LIST:
              //intentional fall through
            case EMBMSHOOK_MSG_ID_GET_ACTIVE: {
                // The client does not have a callback for this response.
                // So send it as an unsol.

                if (msgId == EMBMSHOOK_MSG_ID_GET_ACTIVE && successStatus != SUCCESS) {
                    Log.e(LOG_TAG, "Error received in EMBMSHOOK_MSG_ID_GET_ACTIVE: "
                                    + successStatus);
                    break;
                }
                TmgiListIndication list = new TmgiListIndication(respByteBuf, msgId);
                notifyUnsol(UNSOL_TYPE_ACTIVE_TMGI_LIST, list, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_COVERAGE_STATE:
                //intentional fall through
            case EMBMSHOOK_MSG_ID_GET_COVERAGE: {
                // The client does not have a callback for this response.
                // So send it as an unsol.

                if (msgId == EMBMSHOOK_MSG_ID_GET_COVERAGE && successStatus != SUCCESS) {
                    Log.e(LOG_TAG, "Error received in EMBMSHOOK_MSG_ID_GET_COVERAGE: "
                                    + successStatus);
                    break;
                }

                CoverageState cs = new CoverageState(respByteBuf, msgId);
                notifyUnsol(UNSOL_TYPE_BROADCAST_COVERAGE, cs, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_GET_SIG_STRENGTH: {
                msg.obj = new SigStrengthResponse (successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_SET_TIME: {
                msg.arg1 = successStatus;
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_GET_TIME: {
                msg.obj = new TimeResponse(successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_SIB16:
                // intentional fall through
            case EMBMSHOOK_MSG_ID_GET_SIB16_COVERAGE: {
                if (msgId == EMBMSHOOK_MSG_ID_GET_SIB16_COVERAGE && successStatus != SUCCESS) {
                    Log.e(LOG_TAG, "Error received in EMBMSHOOK_MSG_ID_GET_SIB16_COVERAGE: "
                            + successStatus);
                break;
                }

                Sib16Coverage ind = new Sib16Coverage(respByteBuf);
                notifyUnsol(UNSOL_TYPE_SIB16_COVERAGE, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_GET_ACTIVE_LOG_PACKET_IDS: {
                msg.obj = new ActiveLogPacketIDsResponse (successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_DELIVER_LOG_PACKET: {
                // Clients are not expecting response for deliverLogPacket, so not parsing
                Log.v(LOG_TAG, " deliverLogPacket response successStatus=" + successStatus);
                break;
            }
            case EMBMSHOOK_MSG_ID_CONTENT_DESCRIPTION: {
                // Clients are not expecting response for contentDescription, so not parsing
                Log.v(LOG_TAG, " contentDescription response successStatus=" + successStatus);
                break;
            }
            case EMBMSHOOK_MSG_ID_GET_PLMN_LIST: {
                msg.obj = new GetPLMNListResponse (successStatus, respByteBuf);
                msg.sendToTarget();
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_E911_STATE:
                // intentional fall through
            case EMBMSHOOK_MSG_ID_GET_E911_STATE:{
                E911StateIndication ind = new E911StateIndication(respByteBuf, msgId);
                notifyUnsol(UNSOL_TYPE_E911_STATE, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_OOS_STATE: {
                OosState state = new OosState(respByteBuf);
                notifyUnsol(UNSOL_TYPE_OOS_STATE, state, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_CELL_ID: {
                CellIdIndication ind = new CellIdIndication(respByteBuf);
                notifyUnsol(UNSOL_TYPE_CELL_ID, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_RADIO_STATE: {
                RadioStateIndication ind = new RadioStateIndication(respByteBuf);
                notifyUnsol(UNSOL_TYPE_RADIO_STATE, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_SAI_LIST: {
                SaiIndication ind = new SaiIndication(respByteBuf);
                notifyUnsol(UNSOL_TYPE_SAI_LIST, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL: {
                ContentDescPerObjectControlIndication ind =
                        new ContentDescPerObjectControlIndication(respByteBuf);
                notifyUnsol(UNSOL_TYPE_CONTENT_DESC_PER_OBJ_CONTROL, ind, phoneId);
                break;
            }
            case EMBMSHOOK_MSG_ID_UNSOL_EMBMS_STATUS:
            case EMBMSHOOK_MSG_ID_GET_EMBMS_STATUS: {
                EmbmsStatus status = new EmbmsStatus (respByteBuf, msgId);
                notifyUnsol(UNSOL_TYPE_EMBMS_STATUS, status, phoneId);
                break;
            }
            default:
                Log.e(LOG_TAG, "received unexpected msgId " + msgId);
                break;
        }
    }

    /**
     * Utility to notify clients with an unsol
     */
    private void notifyUnsol(int type, Object payload, int phoneId) {
        UnsolObject obj = new UnsolObject(type, payload, phoneId);
        AsyncResult ar = new AsyncResult (null, obj, null);
        Log.i(LOG_TAG, "Notifying registrants type = " + type);
        mRegistrants.notifyRegistrants(ar);
    }

    /**
     * register for unsolicited notifications
     */
    public void registerForNotifications(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        synchronized (mRegistrants) {
            Log.i(LOG_TAG, "Adding a registrant");
            mRegistrants.add(r);
        }
    }

    /**
     * de-register for unsolicited notifications.
     *
     */
    public void unregisterForNotifications(Handler h) {
        synchronized (mRegistrants) {
            Log.i(LOG_TAG, "Removing a registrant");
            mRegistrants.remove(h);
        }
    }

    /**
     * Enables embms service
     */
    public int enable(int traceId, Message msg, int phoneId) {
        try {
            Log.i(LOG_TAG, "enable called on PhoneId: " + phoneId);
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_ENABLE,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during enable !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Activates the given TMGI.
     */
    public int activateTmgi(int traceId, byte callId, byte[] tmgi, int priority,
                            int[] saiList, int[] earfcnList, Message msg, int phoneId) {
        Log.i(LOG_TAG, "activateTmgi called on PhoneId: " + phoneId);
        TmgiActivateRequest req = new TmgiActivateRequest(traceId, callId, tmgi, priority,
                                                            saiList, earfcnList);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_ACTIVATE,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during activate !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * De-activate a previously activated TMGI
     */
    public int deactivateTmgi(int traceId, byte callId, byte[] tmgi, Message msg, int phoneId) {
        Log.i(LOG_TAG, "deactivateTmgi called on PhoneId: " + phoneId);
        TmgiDeActivateRequest req = new TmgiDeActivateRequest(traceId, tmgi, callId);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_DEACTIVATE,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during deactivate !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Activates and deactivates TMGIs.
     */
    public int actDeactTmgi(int traceId, byte callId, byte[] actTmgi, byte[] deActTmgi,
                            int priority, int[] saiList, int[] earfcnList, Message msg, int phoneId) {

        Log.i(LOG_TAG, "actDeactTmgi called on PhoneId: " + phoneId);
        ActDeactRequest req = new ActDeactRequest(traceId, callId, actTmgi, deActTmgi, priority,
                                                    saiList, earfcnList);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_ACTDEACT,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during activate-deactivate !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get list of available TMGIs
     * This method does not notify the caller by taking a Message
     * Instead the result is indicated through an Unsol
     */
    public int getAvailableTMGIList(int traceId, byte callId, int phoneId) {
        Log.i(LOG_TAG, "getAvailableTMGIList called on PhoneId: " + phoneId);
        GenericRequest req = new GenericRequest(traceId, callId);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_GET_AVAILABLE,
                                            req.getTypes(), req.getItems(), null, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getAvailableTMGIList !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get list of active TMGIs
     * This method does not notify the caller by taking a Message
     * Instead the result is indicated through an Unsol
     */
    public int getActiveTMGIList(int traceId, byte callId, int phoneId) {
        Log.i(LOG_TAG, "getActiveTMGIList called on PhoneId: " + phoneId);
        GenericRequest req = new GenericRequest(traceId, callId);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_GET_ACTIVE,
                                            req.getTypes(), req.getItems(), null, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getActiveTMGIList !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get coverage state
     * This method does not notify the caller by taking a Message
     * Instead the result is indicated through an Unsol
     */
    public int getCoverageState(int traceId, int phoneId) {

        Log.i(LOG_TAG, "getCoverageState called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_COVERAGE,
                                            req.getTypes(), req.getItems(), null, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getActiveTMGIList !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get the signal strength
     *
     */
    public int getSignalStrength(int traceId, Message msg, int phoneId) {
        Log.i(LOG_TAG, "getSignalStrength called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_SIG_STRENGTH,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during enable !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Disable eMBMS service
     */
    public int disable(int traceId, byte callId, Message msg, int phoneId) {
        Log.i(LOG_TAG, "disable called on PhoneId: " + phoneId);
        GenericRequest req = new GenericRequest(traceId, callId);
        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_DISABLE,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during disable !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get the active log packet ID's
     */
    public int getActiveLogPacketIDs(int traceId, int[] supportedLogPacketIdList, Message msg,
            int phoneId) {

        Log.i(LOG_TAG, "getActiveLogPacketIDs called on PhoneId: " + phoneId);
        ActiveLogPacketIDsRequest req = new ActiveLogPacketIDsRequest(traceId,
                supportedLogPacketIdList);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_GET_ACTIVE_LOG_PACKET_IDS,
                                            req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during activate log packet ID's !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Deliver a log packet
     */
    public int deliverLogPacket(int traceId, int logPacketId, byte[] logPacket, int phoneId) {

        Log.i(LOG_TAG, "deliverLogPacket called on PhoneId: " + phoneId);
        DeliverLogPacketRequest req = new DeliverLogPacketRequest(traceId, logPacketId, logPacket);

        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID,
                                            EMBMSHOOK_MSG_ID_DELIVER_LOG_PACKET,
                                            req.getTypes(), req.getItems(), null, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during deliver logPacket !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Utility to convert a byte array into a
     * String of hexadecimal characters
     *
     * input of null returns null
     */
    public static String bytesToHexString(byte[] bytes) {
        if (bytes == null) return null;

        StringBuilder ret = new StringBuilder(2*bytes.length);

        for (int i = 0 ; i < bytes.length ; i++) {
            int b;
            b = 0x0f & (bytes[i] >> 4);
            ret.append("0123456789abcdef".charAt(b));
            b = 0x0f & bytes[i];
            ret.append("0123456789abcdef".charAt(b));
        }

        return ret.toString();
    }

    /**
     * Get UTC time
     */
    public int getTime(int traceId, Message msg, int phoneId) {
        Log.i(LOG_TAG, "getTime called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_TIME,
                    req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getTime !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get Sib16 coverage status from RIL
     */
    public int getSib16CoverageStatus(Message msg, int phoneId) {
        Log.i(LOG_TAG, "getSib16CoverageStatus called on PhoneId: " + phoneId);
        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_SIB16_COVERAGE,
                    msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getSIB16 !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * Get EMBMS status from RIL
     */
    public int getEmbmsStatus(int traceId, int phoneId) {
        Log.i(LOG_TAG, "getEmbmsStatus called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_EMBMS_STATUS,
                    req.getTypes(), req.getItems(), null, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getEmbmsStatus !!!!!!");
            return FAILURE;
        }

        return SUCCESS;
    }

    /**
     * Send the SNTP time
     */
    public int setTime(boolean sntpSuccess, long timeMseconds, long timeStamp, Message msg,
            int phoneId) {
        Log.i(LOG_TAG, "setTime called on PhoneId: " + phoneId);
        byte success = 0;
        if (sntpSuccess) {
            success = 1;
        }
        Log.i(LOG_TAG, "setTime success = " + success + " timeMseconds = " + timeMseconds
                + " timeStamp = " + timeStamp);
        SetTimeRequest req = new SetTimeRequest(success, timeMseconds, timeStamp);
        try {
            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_SET_TIME,
                    req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occured during setTime !!!!!!");
            return FAILURE;
        }

        return SUCCESS;
    }

    /**
     * Send the request to get the current E911 state
     * The result will be indicated to the client through an Unsol.
     */
    public int getE911State(int traceId, Message msg, int phoneId) {
        Log.i(LOG_TAG, "getE911State called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_E911_STATE,
                    req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getE911State !!!!!!");
            return FAILURE;
        }

        return SUCCESS;
    }

    /**
     * Send the contentDescription to provide additional information
     * about the eMBMS content
     */
    public int contentDescription(int traceId, byte callId, byte[] tmgi,int numberOfParameter,
            int[] parameterCode, int[] parameterValue, Message msg, int phoneId) {
        try {
            Log.i(LOG_TAG, "contentDescription called on PhoneId: " + phoneId);
            if (parameterCode == null || parameterValue == null) {
                Log.i(LOG_TAG, "contentDescription: either parameterCode or parameterValue is null"
                        + "parameterCode = " + parameterCode
                        + " parameterValue = " + parameterValue);
                parameterCode = new int[0];
                parameterValue =  new int[0];
            }

            if ((numberOfParameter != parameterCode.length)
                    || (numberOfParameter != parameterValue.length)
                    || (parameterCode.length != parameterValue.length)) {
                Log.e(LOG_TAG, "contentDescription: Invalid input, numberOfParameter = "
                        + numberOfParameter + " parameterCode = " + parameterCode
                        + " parameterValue = " + parameterValue);
                return FAILURE;
            }

            // parameterArray will have paramterCode followed by its corresponding
            // parameterValue.
            int parameterArraySize = numberOfParameter * 2;
            int pointer = 0;
            int [] parameterArray = new int[parameterArraySize];
            for (int i = 0; i < parameterArraySize; i += 2) {
                parameterArray[i] = parameterCode[pointer];
                parameterArray[i + 1] = parameterValue[pointer];
                pointer++;
            }
            Log.i(LOG_TAG, "contentDescription: parameterArray: "
                    + Arrays.toString(parameterArray));

            ContentDescriptionReq req = new ContentDescriptionReq(traceId, callId, tmgi,
                    parameterArray);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_CONTENT_DESCRIPTION,
                    req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during contentDescription !!!!!!");
            return FAILURE;
        }

        return SUCCESS;
    }

    /**
     * Send the plmn list request to RIL.
     */
    public int getPLMNListRequest(int traceId, Message msg, int phoneId) {
        Log.i(LOG_TAG, "getPLMNListRequest called on PhoneId: " + phoneId);
        try {
            BasicRequest req = new BasicRequest(traceId);

            mQmiOemHook.sendQmiMessageAsync(EMBMS_SERVICE_ID, EMBMSHOOK_MSG_ID_GET_PLMN_LIST,
                    req.getTypes(), req.getItems(), msg, phoneId);
        } catch (IOException e) {
            Log.e(LOG_TAG, "IOException occurred during getPLMNListRequest !!!!!!");
            return FAILURE;
        }
        return SUCCESS;
    }

    /**
     * represents an unsol notification.
     */
    public class UnsolObject {
        public int unsolId;
        public Object obj;
        public int phoneId;

        public UnsolObject (int i, Object o, int phone) {
            unsolId = i;
            obj = o;
            phoneId = phone;
        }
    }

    /**
     * Represents the current state of the eMBMS service
     * and the network interface assigned on enable
     */
    public class StateChangeInfo {
        public int state;
        public String ipAddress; // this is valid only when the state is ENABLED
        public int ifIndex; // this is valid only when the state is ENABLED

        public StateChangeInfo(int state, String address, int index) {
            this.state = state;
            this.ipAddress = address;
            this.ifIndex = index;
        }

        public StateChangeInfo(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_UNSOL_STATE_IND_STATE:
                        state = buf.getInt();
                        Log.i(LOG_TAG, "State = " + state);
                        break;
                    case TLV_TYPE_UNSOL_STATE_IND_IP_ADDRESS:
                        byte[] address = new byte[length];
                        for (int i = 0; i < length; i++) {
                            address[i] = buf.get();
                        }
                        ipAddress = new QmiString(address).toString();
                        Log.i(LOG_TAG, "ip Address = " + ipAddress);
                        break;
                    case TLV_TYPE_UNSOL_STATE_IND_IF_INDEX:
                        ifIndex = buf.getInt();
                        Log.i(LOG_TAG, "index = " + ifIndex);
                        break;
                    default:
                        Log.e(LOG_TAG, "StateChangeInfo: Unexpected Type " + type);
                        break;
                }
            }
        }
    }

    /**
     * Utility to extract a list of TMGIs
     * from a ByteBuffer.
     * @param buf The buffer containing the list. The current position
     *            in the buffer should be set to the start of the list
     *            of TMGIs i.e the V in the TLV.
     */
    private byte[] parseTmgi(ByteBuffer buf) {
        int index = 0;
        byte totalTmgis = buf.get();
        byte[] tmgi = new byte[SIZE_OF_TMGI * totalTmgis];

        for (int i = 0; i < totalTmgis; i++) {
            byte tmgiLength = buf.get();
            for (int j = 0; j < tmgiLength; j++) {
                tmgi[index++] = buf.get();
            }
        }
        return tmgi;
    }

    private byte[] parseActiveTmgi(ByteBuffer buf) {
        int index = 0;
        short totalTmgis = buf.getShort();
        byte[] tmgi = new byte[SIZE_OF_TMGI * totalTmgis];

        for (int i = 0; i < totalTmgis; i++) {
            byte tmgiLength = buf.get();
            for (int j = 0; j < tmgiLength; j++) {
                tmgi[index++] = buf.get();
            }
        }
        return tmgi;
    }

    /**
     * Represents the list of available/active TMGIs
     * and session IDs accompanying
     * the available/active TMGI list notification.
     *
     */
    public class TmgiListIndication{
        //list is an optional parameter in case of request-response
        public byte[] list = new byte[0];
        public byte[] sessions = null;
        public int traceId = 0;
        public int code = 0;

        public TmgiListIndication(ByteBuffer buf, short msgId) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_AVAILABLE_IND_TMGI_ARRAY_OR_RESPONSE_CODE:
                            //UNSOL and Resp message share same TLV value;So need to distinguish.
                            //If this buffer corresponds to a request, rather than an unsol,
                            //then this TLV represents the response code.
                            if(msgId == EMBMSHOOK_MSG_ID_GET_AVAILABLE ||
                               msgId == EMBMSHOOK_MSG_ID_GET_ACTIVE) {
                                this.code = buf.getInt();
                                Log.i(LOG_TAG, "response code = " + this.code);
                                break;
                            }
                            //intentional fall through.
                        case TLV_TYPE_GET_AVAILABLE_RESP_TMGI_ARRAY:{
                            this.list = parseTmgi(buf);
                            Log.i(LOG_TAG, "tmgiArray = " + bytesToHexString(this.list));
                            break;
                        }
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "TmgiListIndication: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in TmgiListIndication");
                }
            }
        }
    }

    /**
     * Represents the Out of Service (OOS) State
     * and the TMGIs affected by this state
     */
    public class OosState{
        public int state;
        public byte[] list = null;
        public int traceId = 0;

        public OosState(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_UNSOL_OOS_IND_STATE:
                        state = buf.getInt();
                        Log.i(LOG_TAG, "OOs State = " + state);
                        break;
                    case TLV_TYPE_UNSOL_OOS_IND_TMGI_ARRAY:{
                        this.list = parseTmgi(buf);
                        Log.i(LOG_TAG, "tmgiArray = " + bytesToHexString(this.list));
                        break;
                    }
                    case TLV_TYPE_COMMON_RESP_TRACE_ID:
                        this.traceId = buf.getInt();
                        Log.i(LOG_TAG, "traceId = " + this.traceId);
                        break;
                    default:
                        Log.e(LOG_TAG, "OosState: Unexpected Type " + type);
                        break;
                }
            }
        }
    }

    /**
     * Represents the cell Id Indication
     */
    public class CellIdIndication{
        public String mcc = null;
        public String mnc = null;
        public String id = null;
        public int traceId = 0;

        public CellIdIndication(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());
                    byte[] temp;

                    switch (type) {
                        case TLV_TYPE_UNSOL_CELL_ID_IND_MCC:
                            temp = new byte[length];
                            for (int i = 0; i < length; i++) {
                                temp[i] = buf.get();
                            }
                            mcc = new QmiString(temp).toStringValue();
                            Log.i(LOG_TAG, "MCC = " + mcc);
                            break;
                        case TLV_TYPE_UNSOL_CELL_ID_IND_MNC:{
                            temp = new byte[length];
                            for (int i = 0; i < length; i++) {
                                temp[i] = buf.get();
                            }
                            mnc = new QmiString(temp).toStringValue();
                            Log.i(LOG_TAG, "MNC = " + mnc);
                            break;
                        }
                        case TLV_TYPE_UNSOL_CELL_ID_IND_CID:{
                            //CellId should be with fixed length of 28 bits.
                            id = String.format("%7s", Integer.toHexString(buf.getInt())).
                                    replace(' ', '0');
                            Log.i(LOG_TAG, "CellId = " + id);
                            break;
                        }
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "CellIdIndication: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for CellIdIndication");
                }
            }
        }
    }

    /**
     * Represents the Radio State Indication
     */
    public class RadioStateIndication{
        public int state = 0;
        public int traceId = 0;

        public RadioStateIndication(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_RADIO_STATE:
                            state = buf.getInt();
                            Log.i(LOG_TAG, "radio = " + state);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "RadioStateIndication: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for RadioStateIndication");
                }
            }
        }
    }

    /**
     * Represents the Service Area ID (SAI) List Indication
     */
    public class SaiIndication{
        public int[] campedSaiList = null;
        public int[] numSaiPerGroupList = null;
        public int[] availableSaiList = null;
        public int traceId = 0;

        public SaiIndication(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = buf.get();
                    short length = buf.getShort();
                    int[] list = null;
                    byte listLength;

                    switch (type) {
                        case TLV_TYPE_UNSOL_SAI_IND_CAMPED_SAI_LIST:
                            listLength = buf.get();
                            list = new int[listLength];
                            for (int i = 0; i < listLength; i++) {
                                list[i] = buf.getInt();
                            }
                            this.campedSaiList = list;
                            Log.i(LOG_TAG, "Camped list = " + Arrays.toString(this.campedSaiList));
                            break;
                        case TLV_TYPE_UNSOL_SAI_IND_SAI_PER_GROUP_LIST:
                            listLength = buf.get();
                            list = new int[listLength];
                            for (int i = 0; i < listLength; i++) {
                                list[i] = buf.getInt();
                            }
                            this.numSaiPerGroupList = list;
                            Log.i(LOG_TAG, "Number of SAI per group list = "
                                    + Arrays.toString(this.numSaiPerGroupList));
                            break;
                        case TLV_TYPE_UNSOL_SAI_IND_AVAILABLE_SAI_LIST:
                            short availableLength = buf.getShort();
                            list = new int[availableLength];
                            for (int i = 0; i < availableLength; i++) {
                                list[i] = buf.getInt();
                            }
                            this.availableSaiList = list;
                            Log.i(LOG_TAG, "Available SAI list = "
                                    + Arrays.toString(this.availableSaiList));
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "SaiIndication: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for SaiIndication");
                }
            }
        }
    }

    /**
     * Represents the EMBMS Coverage State
     */
    public class CoverageState{
        public int status;
        public int state;
        public int traceId = 0;
        //response code is present only in the coverage state request-response
        public int code = 0;

        public CoverageState(ByteBuffer buf, short msgId) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_COVERAGE_IND_STATE_OR_RESPONSE_CODE:
                            //UNSOL and Resp message share same TLV value; So need to distinguish.
                            //If this buffer corresponds to a request, rather than an unsol,
                            //then this TLV represents the response code.
                            if(msgId == EMBMSHOOK_MSG_ID_GET_COVERAGE) {
                                this.code = buf.getInt();
                                Log.i(LOG_TAG, "response code = " + this.code);
                                break;
                            }
                            //intentional fall through.
                        case TLV_TYPE_GET_COVERAGE_STATE_RESP_STATE:
                            state = buf.getInt();
                            Log.i(LOG_TAG, "Coverage State = " + state);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "CoverageState: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in CoverageState");
                }
            }
        }
    }

    public class Sib16Coverage {
        public boolean inCoverage = false;

        public Sib16Coverage(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_SIB16:
                            byte coverage = buf.get();
                            if (coverage == 1) {
                                this.inCoverage = true;
                            }
                            Log.i(LOG_TAG, "Unsol SIB16 coverage status = " + this.inCoverage);
                            break;
                        default:
                            Log.e(LOG_TAG, "Sib16Coverage: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for Sib16Coverage");
                }
            }
        }
    }

    public class E911StateIndication {
        public int state;
        public int traceId = 0;
        public int code;

        public E911StateIndication(ByteBuffer buf, short msgId) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_E911_STATE_OR_RESPONSE_CODE:
                            //UNSOL and Resp message share same TLV value; So need to distinguish.
                            //If this buffer corresponds to a request, rather than an unsol,
                            //then this TLV represents the response code.
                            if(msgId == EMBMSHOOK_MSG_ID_GET_E911_STATE) {
                                this.code = buf.getInt();
                                Log.i(LOG_TAG, "response code = " + this.code);
                                break;
                            }
                            //intentional fall through
                        case TLV_TYPE_GET_E911_RESP_STATE:
                            state = buf.getInt();
                            Log.i(LOG_TAG, "E911 State = " + state);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "E911 State: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for E911 Notification");
                }
            }
        }
    }

    public class ContentDescPerObjectControlIndication {
        public int traceId = 0;
        public byte[] tmgi = null;
        public int perObjectContentControl;
        public int perObjectStatusControl;

        public ContentDescPerObjectControlIndication(ByteBuffer buf) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_TMGI:
                            byte tmgiLength = buf.get();
                            byte[] tmgi = new byte[tmgiLength];
                            for (int i = 0; i < tmgiLength; i++) {
                                tmgi[i] = buf.get();
                            }
                            this.tmgi = tmgi;
                            Log.i(LOG_TAG, "tmgi = " + bytesToHexString(this.tmgi));
                            break;
                        case TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_CONTENT_CONTROL:
                            perObjectContentControl = buf.getInt();
                            Log.i(LOG_TAG, "perObjectContentControl = " + perObjectContentControl);
                            break;
                        case TLV_TYPE_UNSOL_CONTENT_DESC_PER_OBJ_CONTROL_STATUS_CONTROL:
                            perObjectStatusControl = buf.getInt();
                            Log.i(LOG_TAG, "perObjectStatusControl = " + perObjectStatusControl);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "ContentDescPerObjectControl: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for"
                            + "ContentDescPerObjectControl Notification");
                }
            }
        }
    }

    /**
     * Represents a response to enabling eMBMS service
     *
     */
    public class EnableResponse {
        public int status;
        public int traceId;
        public int code = 0;
        // The following members are valid only if status == 0, i.e. success
        public byte callId = 0;
        public String interfaceName = null;
        public int ifIndex = 0;

        public EnableResponse(int error, ByteBuffer buf) {
            this.status = error;

           // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_COMMON_RESP_CALL_ID:
                        this.callId = buf.get();
                        Log.i(LOG_TAG, "callid = " + this.callId);
                        break;
                    case TLV_TYPE_ENABLE_RESP_IFNAME:
                        byte[] name = new byte[length];
                        for (int i = 0; i < length; i++) {
                            name[i] = buf.get();
                        }
                        interfaceName = new QmiString(name).toStringValue();
                        Log.i(LOG_TAG, "ifName = " + interfaceName);
                        break;
                    case TLV_TYPE_ENABLE_RESP_IF_INDEX:
                        this.ifIndex = buf.getInt();
                        Log.i(LOG_TAG, "ifIndex = " + this.ifIndex);
                        break;
                    case TLV_TYPE_COMMON_RESP_CODE:
                        this.code = buf.getInt();
                        Log.i(LOG_TAG, "code = " + this.code);
                        break;
                    case TLV_TYPE_COMMON_RESP_TRACE_ID:
                        this.traceId = buf.getInt();
                        Log.i(LOG_TAG, "traceId = " + this.traceId);
                        break;
                    default:
                        Log.e(LOG_TAG, "EnableResponse: Unexpected Type " + type);
                        break;
                }
            }
        }
    }

    /**
     * Represents a response to disabling eMBMS service
     *
     */
    public class DisableResponse {
        public int status;
        public int code = 0;
        public int traceId;
        // The following members are valid only if error == 0, i.e. success
        public byte callId = 0;

        public DisableResponse(int error, ByteBuffer buf) {
            this.status = error;

            // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_COMMON_RESP_CALL_ID:
                        this.callId = buf.get();
                        Log.i(LOG_TAG, "callid = " + this.callId);
                        break;
                    case TLV_TYPE_COMMON_RESP_CODE:
                        this.code = buf.getInt();
                        Log.i(LOG_TAG, "code = " + this.code);
                        break;
                    case TLV_TYPE_COMMON_RESP_TRACE_ID:
                        this.traceId = buf.getInt();
                        Log.i(LOG_TAG, "traceId = " + this.traceId);
                        break;
                    default:
                        Log.e(LOG_TAG, "DisableResponse: Unexpected Type " + type);
                        break;
                }
            }
        }
    }


    /**
     * A common representation of a request that only has the trace parameter
     */
    public class BasicRequest extends BaseQmiStructType {
        public QmiInteger traceId;

        public BasicRequest(int trace) {
            this.traceId = new QmiInteger(trace);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID};
        }
    }

    /**
     * Represents a any request that only has the following parameters
     * - trace ID
     * - call ID
     */
    public class GenericRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiByte callId;

        public GenericRequest(int trace, byte callId) {
            this.traceId = new QmiInteger(trace);
            this.callId = new QmiByte(callId);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, callId};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID, TLV_TYPE_COMMON_REQ_CALL_ID};
        }
    }

    /**
     * Utility to convert a byte array to a QmiArray of QmiBytes
     * @param vSize Size of length within V field of TLV
     */
    private QmiArray<QmiByte> byteArrayToQmiArray (short vSize, byte[] arr){
        QmiByte[] qmiByteArray = new QmiByte[arr.length];
        for(int i = 0; i < arr.length; i++) {
            qmiByteArray[i] = new QmiByte(arr[i]);
        }
        return new QmiArray<QmiByte>(qmiByteArray, QmiByte.class, vSize);
    }


    /**
     * Utility to convert an int array to a QmiArray of QmiInteger
     * @param vSize Size of length within V field of TLV
     */
    private QmiArray<QmiInteger> intArrayToQmiArray (short vSize, int[] arr){
        int length = (arr == null)? 0 : arr.length;

        QmiInteger[] qmiIntArray = new QmiInteger[length];
        for(int i = 0; i < length; i++) {
            qmiIntArray[i] = new QmiInteger(arr[i]);
        }
        return new QmiArray<QmiInteger>(qmiIntArray, QmiInteger.class, vSize);
    }

    /**
     * Utility to convert an int array to a QmiArray of QmiInteger
     * @param vSize Size of length within V field of TLV
     * @numOfElements The number of elements in one set of V in TLV.
     *    For example, the contentDescription() request has parameterCode
     *    and parameterValue pair in V in the TLV. One parameterCode and
     *    prameterValue pair is considered as one set. Therefore, the
     *    numOfElements is 2.
     */
    private QmiArray<QmiInteger> intArrayToQmiArray (short vSize, int[] arr, short numOfElements){
        int length = (arr == null)? 0 : arr.length;

        QmiInteger[] qmiIntArray = new QmiInteger[length];
        for(int i = 0; i < length; i++) {
            qmiIntArray[i] = new QmiInteger(arr[i]);
        }
        return new QmiArray<QmiInteger>(qmiIntArray, QmiInteger.class, vSize, numOfElements);
    }

    /**
     * Represents a request to activate a TMGI
     *
     */
    public class TmgiActivateRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiArray<QmiByte> tmgi;
        public QmiByte callId;
        public QmiInteger priority;
        public QmiArray<QmiInteger> saiList;
        public QmiArray<QmiInteger> earfcnList;

        public TmgiActivateRequest(int trace, byte callId, byte[] tmgi,
                                   int priority, int[] saiList, int[] earfcnList) {
            this.traceId = new QmiInteger(trace);
            this.callId = new QmiByte(callId);
            this.priority = new QmiInteger(priority);
            this.tmgi = byteArrayToQmiArray(ONE_BYTE, tmgi);
            this.saiList = intArrayToQmiArray(ONE_BYTE, saiList);
            this.earfcnList = intArrayToQmiArray(ONE_BYTE, earfcnList);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, callId, tmgi, priority, saiList, earfcnList};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_COMMON_REQ_CALL_ID,
                                TLV_TYPE_ACTIVATE_REQ_TMGI,
                                TLV_TYPE_ACTIVATE_REQ_PRIORITY,
                                TLV_TYPE_ACTIVATE_REQ_SAI_LIST,
                                TLV_TYPE_ACTIVATE_REQ_EARFCN_LIST};
        }
    }

    /**
     * Represents a request to activate and deactivate TMGI
     *
     */
    public class ActDeactRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiArray<QmiByte> actTmgi;
        public QmiArray<QmiByte> deActTmgi;
        public QmiByte callId;
        public QmiInteger priority;
        public QmiArray<QmiInteger> saiList;
        public QmiArray<QmiInteger> earfcnList;

        public ActDeactRequest(int trace, byte callId, byte[] actTmgi, byte[] deActTmgi,
                                   int priority, int[] saiList, int[] earfcnList) {
            this.traceId = new QmiInteger(trace);
            this.callId = new QmiByte(callId);
            this.priority = new QmiInteger(priority);
            this.actTmgi = byteArrayToQmiArray(ONE_BYTE, actTmgi);
            this.deActTmgi = byteArrayToQmiArray(ONE_BYTE, deActTmgi);
            this.saiList = intArrayToQmiArray(ONE_BYTE, saiList);
            this.earfcnList = intArrayToQmiArray(ONE_BYTE, earfcnList);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, callId, actTmgi, deActTmgi, priority,
                    saiList, earfcnList};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_COMMON_REQ_CALL_ID,
                                TLV_TYPE_ACTDEACTIVATE_REQ_ACT_TMGI,
                                TLV_TYPE_ACTDEACTIVATE_REQ_DEACT_TMGI,
                                TLV_TYPE_ACTDEACTIVATE_REQ_PRIORITY,
                                TLV_TYPE_ACTDEACTIVATE_REQ_SAI_LIST,
                                TLV_TYPE_ACTDEACTIVATE_REQ_EARFCN_LIST};
        }
    }

    /**
     * Represents a request to deactivate a TMGi
     *
     */
    public class TmgiDeActivateRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiArray<QmiByte> tmgi;
        public QmiByte callId;

        public TmgiDeActivateRequest(int trace, byte [] tmgi, byte callId) {
            this.traceId = new QmiInteger(trace);
            this.tmgi = byteArrayToQmiArray(ONE_BYTE, tmgi);
            this.callId = new QmiByte(callId);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, callId, tmgi};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_COMMON_REQ_CALL_ID,
                                TLV_TYPE_DEACTIVATE_REQ_TMGI};
        }
    }

    /**
     * Represents a request for setTime
     */
    public class SetTimeRequest extends BaseQmiStructType {
        public QmiByte sntpSuccess;
        public QmiLong timeMseconds;
        public QmiLong timeStamp;

        public SetTimeRequest(byte sntpSuccess, long timeMseconds, long timeStamp) {
            this.sntpSuccess = new QmiByte(sntpSuccess);
            this.timeMseconds = new QmiLong(timeMseconds);
            this.timeStamp = new QmiLong(timeStamp);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {
                    sntpSuccess, timeMseconds, timeStamp
            };
        }

        @Override
        public short[] getTypes() {
            return new short[] {
                    TLV_TYPE_SET_TIME_REQ_SNTP_SUCCESS, TLV_TYPE_SET_TIME_REQ_TIME_MSECONDS,
                    TLV_TYPE_SET_TIME_REQ_TIME_STAMP
            };
        }
    }

    /**
     * Represents a request to get active log packet Id's
     *
     */
    public class ActiveLogPacketIDsRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiArray<QmiInteger> supportedLogPacketIdList;

        public ActiveLogPacketIDsRequest(int trace, int[] supportedLogPacketIdList) {
            this.traceId = new QmiInteger(trace);
            this.supportedLogPacketIdList = intArrayToQmiArray(TWO_BYTES, supportedLogPacketIdList);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, supportedLogPacketIdList};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_ACTIVELOGPACKETID_REQ_PACKET_ID_LIST};
        }
    }

    /**
     * Represents a request to deliver a log packet
     *
     */
    public class DeliverLogPacketRequest extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiInteger logPacketId;
        public QmiArray<QmiByte> logPacket;

        public DeliverLogPacketRequest(int trace, int logPacketId, byte[] logPacket) {
            this.traceId = new QmiInteger(trace);
            this.logPacketId = new QmiInteger(logPacketId);
            this.logPacket = byteArrayToQmiArray(TWO_BYTES, logPacket);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, logPacketId, logPacket};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_DELIVERLOGPACKET_REQ_PACKET_ID,
                                TLV_TYPE_DELIVERLOGPACKET_REQ_LOG_PACKET};
        }
    }

    /**
     * Represents a request to send contentDescription
     *
     */
    public class ContentDescriptionReq extends BaseQmiStructType {
        public QmiInteger traceId;
        public QmiByte callId;
        public QmiArray<QmiByte> tmgi;
        public QmiArray<QmiInteger> parameterArray;

        public ContentDescriptionReq(int trace, byte callId, byte[] tmgi, int[] parameterArray) {
            this.traceId = new QmiInteger(trace);
            this.callId = new QmiByte(callId);
            this.tmgi = byteArrayToQmiArray(ONE_BYTE, tmgi);
            // The parameterCode and parameterValue pair is considered as one set .
            // Therefore, the number of elements in one set of V of the TLV for
            // paramterArray will be 2.
            short numberOfElements = 2;
            this.parameterArray = intArrayToQmiArray(ONE_BYTE, parameterArray, numberOfElements);
        }

        @Override
        public BaseQmiItemType[] getItems() {
            return new BaseQmiItemType[] {traceId, callId, tmgi, parameterArray};
        }

        @Override
        public short[] getTypes() {
            return new short[] {TLV_TYPE_COMMON_REQ_TRACE_ID,
                                TLV_TYPE_COMMON_REQ_CALL_ID,
                                TLV_TYPE_CONTENT_DESCRIPTION_REQ_TMGI,
                                TLV_TYPE_CONTENT_DESCRIPTION_REQ_PARAMETER_ARRAY};
        }
    }

    /**
     * Response class representing the async response to an
     * activate or de-activate TMGI request.
     *
     */
    public class TmgiResponse {
        public int status;
        public int code = 0;
        public int traceId = 0;
        public byte[] tmgi = null;

        public TmgiResponse (int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());

                switch (type) {
                    case TLV_TYPE_ACTIVATE_RESP_TMGI:
                        byte tmgiLength = buf.get();
                        byte[] tmgi = new byte[tmgiLength];
                        for (int i = 0; i < tmgiLength; i++) {
                            tmgi[i] = buf.get();
                        }
                        this.tmgi = tmgi;
                        Log.i(LOG_TAG, "tmgi = " + bytesToHexString(this.tmgi));
                        break;
                    case TLV_TYPE_COMMON_RESP_CALL_ID:
                        byte id = buf.get();
                        Log.i(LOG_TAG, "callid = " + id);
                        break;
                    case TLV_TYPE_COMMON_RESP_CODE:
                        this.code = buf.getInt();
                        Log.i(LOG_TAG, "code = " + this.code);
                        break;
                    case TLV_TYPE_COMMON_RESP_TRACE_ID:
                        this.traceId = buf.getInt();
                        Log.i(LOG_TAG, "traceId = " + this.traceId);
                        break;
                    default:
                        Log.e(LOG_TAG, "TmgiResponse: Unexpected Type " + type);
                        break;
                }
            }
        }
    }

    /**
     * Response class representing the async response to
     * get the signal strength
     *
     */
    public class SigStrengthResponse {
        public int status;
        public int code = 0;
        public int traceId = 0;
        //The following are optional fields
        public float[] snr = null;
        public int[] mbsfnAreaId = null;
        public float[] esnr = null;       //excessSNR
        public int[] tmgiPerMbsfn = null;
        public byte[] tmgilist = null;

        public SigStrengthResponse (int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = buf.get();
                    short length = buf.getShort();

                    switch (type) {
                        case TLV_TYPE_GET_SIG_STRENGTH_RESP_SNR:
                            byte snrLength = buf.get();
                            float[] snrArray = new float[snrLength];
                            for (int i = 0; i < snrLength; i++) {
                                snrArray[i] = buf.getFloat();
                            }
                            this.snr = snrArray;
                            Log.i(LOG_TAG, "SNR = " + Arrays.toString(this.snr));
                            break;
                        case TLV_TYPE_GET_SIG_STRENGTH_RESP_MBSFN_AREA_ID:
                            byte mbsfnLength = buf.get();
                            int[] mbsfnArray = new int[mbsfnLength];
                            for(int i = 0; i < mbsfnLength; i++) {
                                mbsfnArray[i] = buf.getInt();
                            }
                            this.mbsfnAreaId = mbsfnArray;
                            Log.i(LOG_TAG, "MBSFN_Area_ID = " + Arrays.toString(this.mbsfnAreaId));
                            break;
                        case TLV_TYPE_GET_SIG_STRENGTH_RESP_EXCESS_SNR:
                            byte esnrLength = buf.get();
                            float[] esnrArray = new float[esnrLength];
                            for(int i = 0; i < esnrLength; i++) {
                                esnrArray[i] = buf.getFloat();
                            }
                            this.esnr = esnrArray;
                            Log.i(LOG_TAG, "EXCESS SNR = " +  Arrays.toString(this.esnr));
                            break;
                        case TLV_TYPE_GET_SIG_STRENGTH_RESP_NUMBER_OF_TMGI_PER_MBSFN:
                            byte tmgiPerMbsfnLength = buf.get();
                            int[] tmgiPerMbsfnArray = new int[tmgiPerMbsfnLength];
                            for(int i = 0; i < tmgiPerMbsfnLength; i++) {
                                tmgiPerMbsfnArray[i] = buf.getInt();
                            }
                            this.tmgiPerMbsfn = tmgiPerMbsfnArray;
                            Log.i(LOG_TAG, "NUMBER OF TMGI PER MBSFN = " +
                                    Arrays.toString(this.tmgiPerMbsfn));
                            break;
                        case TLV_TYPE_GET_SIG_STRENGTH_RESP_ACTIVE_TMGI_LIST:
                            this.tmgilist = parseActiveTmgi(buf);
                            Log.i(LOG_TAG, "tmgiArray = " + bytesToHexString(this.tmgilist));
                            break;
                        case TLV_TYPE_COMMON_RESP_CODE:
                            this.code = buf.getInt();
                            Log.i(LOG_TAG, "code = " + this.code);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "SigStrengthResponse: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in SigStrengthResponse");
                }
            }
            //this is for optional fields.
            if(this.snr == null) this.snr = new float[0];
            if(this.esnr == null) this.esnr = new float[0];
            if(this.tmgiPerMbsfn == null) this.tmgiPerMbsfn = new int[0];
            if(this.mbsfnAreaId == null) this.mbsfnAreaId = new int[0];
            if(this.tmgilist == null ) this.tmgilist = new byte[0];
         }
    }

    /**
     * Response class representing the async response to an
     * activate - de-activate TMGI request.
     *
     */
    public class ActDeactResponse {
        public int status;
        public short actCode = 0;
        public short deactCode = 0;
        public int traceId = 0;
        public byte[] actTmgi = null;
        public byte[] deactTmgi = null;

        public ActDeactResponse (int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                int type = PrimitiveParser.toUnsigned(buf.get());
                int length = PrimitiveParser.toUnsigned(buf.getShort());
                byte tmgiLength = 0;
                byte[] tmgi = null;

                switch (type) {
                    case TLV_TYPE_ACTDEACTIVATE_RESP_ACTTMGI:
                        tmgiLength = buf.get();
                        tmgi = new byte[tmgiLength];
                        for (int i = 0; i < tmgiLength; i++) {
                            tmgi[i] = buf.get();
                        }
                        this.actTmgi = tmgi;
                        Log.i(LOG_TAG, "Act tmgi = " + bytesToHexString(this.actTmgi));
                        break;
                    case TLV_TYPE_ACTDEACTIVATE_RESP_DEACTTMGI:
                        tmgiLength = buf.get();
                        tmgi = new byte[tmgiLength];
                        for (int i = 0; i < tmgiLength; i++) {
                            tmgi[i] = buf.get();
                        }
                        this.deactTmgi = tmgi;
                        Log.i(LOG_TAG, "Deact tmgi = " + bytesToHexString(this.deactTmgi));
                        break;
                    case TLV_TYPE_COMMON_RESP_CALL_ID:
                        byte id = buf.get();
                        Log.i(LOG_TAG, "callid = " + id);
                        break;
                    case TLV_TYPE_ACTDEACTIVATE_RESP_ACT_CODE:
                        this.actCode = buf.getShort();
                        Log.i(LOG_TAG, "Act code = " + this.actCode);
                        break;
                    case TLV_TYPE_ACTDEACTIVATE_RESP_DEACT_CODE:
                        this.deactCode = buf.getShort();
                        Log.i(LOG_TAG, "Deact code = " + this.deactCode);
                        break;
                    case TLV_TYPE_COMMON_RESP_TRACE_ID:
                        this.traceId = buf.getInt();
                        Log.i(LOG_TAG, "traceId = " + this.traceId);
                        break;
                    default:
                        Log.e(LOG_TAG, "TmgiResponse: Unexpected Type " + type);
                        break;
                }
            }
        }
    }


    /**
     * Response class representing the async response to getTime request.
     */
    public class TimeResponse {
        public int status;
        public int code = 0;
        public long timeMseconds = 0;
        //this is true if the daylight or leapsec or time offset is available
        public boolean additionalInfo = false;
        public boolean dayLightSaving = false;
        public byte leapSeconds = 0;
        public int traceId = 0;
        public long localTimeOffset = 0;

        public TimeResponse(int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_GET_TIME_RESP_TIME_MSECONDS:
                            this.timeMseconds = buf.getLong();
                            Log.i(LOG_TAG, "timeMseconds = " + this.timeMseconds);
                            break;
                        case TLV_TYPE_GET_TIME_RESP_DAY_LIGHT_SAVING:
                            this.additionalInfo = true;
                            byte isdayLightSaving = buf.get();
                            if (isdayLightSaving == 1) {
                                this.dayLightSaving = true;
                            }
                            Log.i(LOG_TAG, "dayLightSaving = " + this.dayLightSaving);
                            break;
                        case TLV_TYPE_GET_TIME_RESP_LEAP_SECONDS:
                            this.additionalInfo = true;
                            this.leapSeconds = buf.get();
                            Log.i(LOG_TAG, "leapSeconds = " + this.leapSeconds);
                            break;
                        case TLV_TYPE_GET_TIME_RESP_LOCAL_TIME_OFFSET:
                            this.additionalInfo = true;
                            this.localTimeOffset = (long) buf.get();
                            Log.i(LOG_TAG, "localTimeOffset = " + this.localTimeOffset);
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        case TLV_TYPE_COMMON_RESP_CODE:
                            this.code = buf.getInt();
                            Log.i(LOG_TAG, "code = " + this.code);
                            break;
                        default:
                            Log.e(LOG_TAG, "TimeResponse: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in TimeResponse");
                }
            }
            Log.i(LOG_TAG, "additionalInfo = " + this.additionalInfo);
         }

        public TimeResponse(int traceId, int status, long timeMseconds, boolean additonalInfo,
                long localTimeOffset, boolean dayLightSaving, byte leapSeconds) {
            this.status = status;
            this.traceId = traceId;
            this.code = SUCCESS;
            this.timeMseconds = timeMseconds;
            this.localTimeOffset = localTimeOffset;
            this.additionalInfo = additionalInfo;
            this.dayLightSaving = dayLightSaving;
            this.leapSeconds = leapSeconds;

            Log.i(LOG_TAG, "TimeResponse: traceId = " + this.traceId + " code = " + this.code
                    + " timeMseconds = " + this.timeMseconds + "additionalInfo = "
                    + this.additionalInfo + " localTimeOffset = " + this.localTimeOffset
                    + " dayLightSaving = " + this.dayLightSaving + " leapSeconds = "
                    + this.leapSeconds);
        }
    }


    /**
     * Response class representing the async response to
     * get active logPacket ID's request
     *
     */

    public class ActiveLogPacketIDsResponse {
        public int status;
        public int traceId = 0;
        public int[] activePacketIdList = null;

        public ActiveLogPacketIDsResponse (int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());
                    byte packetIdLength = 0;

                    switch (type) {
                        case TLV_TYPE_ACTIVELOGPACKETID_RESP_PACKET_ID_LIST:
                            short logPacketIdLength = buf.getShort();
                            int[] activeLogPacketIdListArray = new int[logPacketIdLength];
                            for(int i = 0; i < logPacketIdLength; i++) {
                                activeLogPacketIdListArray[i] = buf.getInt();
                            }
                            this.activePacketIdList = activeLogPacketIdListArray;
                            Log.i(LOG_TAG, "Active log packet Id's = " +
                                    Arrays.toString(this.activePacketIdList));
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "ActiveLogPacketIDsResponse: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in "+
                            "ActiveLogPacketIDsResponse");

                }
            }
        }
    }

    /**
     * Response class representing the async response to
     * get plmn list request
     *
     */

    public class GetPLMNListResponse {
        public int status;
        public int traceId = 0;
        public byte[] plmnList = null;

        public GetPLMNListResponse (int status, ByteBuffer buf) {
            this.status = status;
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());

                    switch (type) {
                        case TLV_TYPE_GET_PLMN_LIST_RESP_PLMN_LIST:
                            byte plmnLength = buf.get();
                            byte[] plmnList = new byte[plmnLength];
                            for (int i = 0; i < plmnLength; i++) {
                                plmnList[i] = buf.get();
                            }
                            this.plmnList = plmnList;
                            Log.i(LOG_TAG, "plmnList = " + bytesToHexString(this.plmnList));
                            break;
                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "GetPLMNListResponse: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Invalid format of byte buffer received in "+
                            "GetPLMNListResponse");

                }
            }
        }
    }

    public class EmbmsStatus {
        public boolean embmsStatus = false;
        public int traceId = 0;
        private static final int TYPE_EMBMS_STATUS = 1000;

        public EmbmsStatus(ByteBuffer buf, int msgId) {
            // Parse through the TLVs
            while (buf.hasRemaining()) {
                try {
                    int type = PrimitiveParser.toUnsigned(buf.get());
                    int length = PrimitiveParser.toUnsigned(buf.getShort());
                    if ( type == TLV_TYPE_UNSOL_EMBMS_STATUS &&
                            msgId == EMBMSHOOK_MSG_ID_UNSOL_EMBMS_STATUS) {
                        type = TYPE_EMBMS_STATUS;
                    }
                    switch (type) {
                        case TLV_TYPE_GET_EMBMS_STATUS_RESP:
                        case TYPE_EMBMS_STATUS:
                            byte status = buf.get();
                            Log.i(LOG_TAG, "Unsol embmsStatus received = " + status);
                            if (status == 1) {
                                this.embmsStatus = true;
                            }
                            Log.i(LOG_TAG, "Unsol embmsStatus = " + this.embmsStatus);
                            break;

                        case TLV_TYPE_COMMON_RESP_TRACE_ID:
                            this.traceId = buf.getInt();
                            Log.i(LOG_TAG, "traceId = " + this.traceId);
                            break;
                        default:
                            Log.e(LOG_TAG, "embmsStatus: Unexpected Type " + type);
                            break;
                    }
                } catch (BufferUnderflowException e) {
                    Log.e(LOG_TAG, "Unexpected buffer format when parsing for embmsStatus");
                }
            }
        }
    }

}

