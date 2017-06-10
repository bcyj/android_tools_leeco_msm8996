/******************************************************************************
 * @file    QcRilOemHook.java
 * @brief   Provides the library functions to Android Applications to add
 *          Qualcomm OEM Header to Qualcomm RIL OEM Hook Messages and send
 *          them using the RIL OEM Hook interface.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2009 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.qcrilhook;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.BufferUnderflowException;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.util.Log;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.uicc.IccUtils;
import com.qualcomm.qcrilhook.IOemHookCallback;
import com.qualcomm.qcrilmsgtunnel.IQcrilMsgTunnel;

public class QcRilHook implements IQcRilHook {
    private final static String LOG_TAG = "QC_RIL_OEM_HOOK";

    private final String mOemIdentifier = "QOEMHOOK";

    private final static int RESPONSE_BUFFER_SIZE = 2048;
    private final static int MAX_REQUEST_BUFFER_SIZE = 1024;

    private final static int INT_SIZE = 4;

    private final static int BYTE_SIZE = 1;
    private static final int AVOIDANCE_BUFF_LEN = 164;

    private static final int DEFAULT_PHONE = 0;

    // Header consists of OEM Identifier String, Request Id(int) and Request
    // Size (int)
    private final int mHeaderSize = mOemIdentifier.length() + 2 * INT_SIZE;

    /*
     * According to C.S0016-C_v2.0 3.3.6
     * SPC Value range is from 0 to 999,999
     */
    private final static int MAX_SPC_LEN = 6;

    private final static int MAX_PDC_ID_LEN = 124;

    private IQcrilMsgTunnel mService = null;

    private static RegistrantList mRegistrants;

    private Context mContext;
    private boolean mBound = false;
    private QcRilHookCallback mQcrilHookCb = null;

    private final String ENCODING = "ISO-8859-1";
    public static final String ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW =
            "android.intent.action.ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW";

    public static final String QCRIL_MSG_TUNNEL_SERVICE_NAME =
            "com.qualcomm.qcrilmsgtunnel.QcrilMsgTunnelService";

    public static final String QCRIL_MSG_TUNNEL_PACKAGE_NAME = "com.qualcomm.qcrilmsgtunnel";

    @Deprecated
    public QcRilHook(Context context) {
        this(context, null);
    }

    public QcRilHook(Context context, QcRilHookCallback cb) {
        super();
        mQcrilHookCb = cb;
        mRegistrants = new RegistrantList();

        mContext = context;
        Intent intent = new Intent();
        intent.setClassName(QCRIL_MSG_TUNNEL_PACKAGE_NAME, QCRIL_MSG_TUNNEL_SERVICE_NAME);

        Log.d(LOG_TAG, "Starting QcrilMsgTunnel Service");
        mContext.startService(intent);

        mContext.bindService(intent, mQcrilMsgTunnelConnection, Context.BIND_AUTO_CREATE);
        Log.d(LOG_TAG, "The QcrilMsgTunnelService will be connected soon ");

        try {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW);
            mContext.registerReceiver(mIntentReceiver, filter);
            Log.d(LOG_TAG, "Registering for intent ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW");
        } catch (Exception e) {
            Log.e(LOG_TAG, "Uncaught Exception while while registering " +
                    "ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW intent. Reason: " + e);
        }
    }

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW)) {
                int response_id = 0;
                Log.d(LOG_TAG, "Received Broadcast Intent ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW");

                /* Get Intent payload */
                byte[] payload = intent.getByteArrayExtra("payload");
                int instanceId = intent.getIntExtra(QmiOemHookConstants.INSTANCE_ID, 0);

                if (payload != null) {
                    /* Parse QCRILHOOK Unsol Response Header and get response_id */
                    if (payload.length < mHeaderSize) {
                        Log.e(LOG_TAG, "UNSOL_RESPONSE_OEM_HOOK_RAW incomplete header");
                        Log.e(LOG_TAG, "Expected " + mHeaderSize + " bytes. Received "
                                + payload.length + " bytes.");
                        return;
                    } else {
                        ByteBuffer response = createBufferWithNativeByteOrder(payload);
                        byte[] oem_id_bytes = new byte[mOemIdentifier.length()];
                        response.get(oem_id_bytes);
                        String oem_id_str = new String(oem_id_bytes);
                        Log.d(LOG_TAG, "Oem ID in QCRILHOOK UNSOL RESP is " + oem_id_str);
                        if (!oem_id_str.equals(mOemIdentifier)) {
                            Log.w(LOG_TAG, "Incorrect Oem ID in QCRILHOOK UNSOL RESP. Expected "
                                    + mOemIdentifier + ". Received " + oem_id_str);
                            return;
                        }

                        int remainingSize = payload.length - mOemIdentifier.length();
                        if (remainingSize > 0) {
                            byte[] remainingPayload = new byte[remainingSize];

                            response.get(remainingPayload);
                            Message msg = Message.obtain();
                            msg.obj = remainingPayload;
                            msg.arg1 = instanceId;
                            /*
                             * Send the remaining payload in AsyncResult.result
                             * to appropriate handlers
                             */
                            //AsyncResult ar = new AsyncResult(null, remainingPayload, null);
                            AsyncResult ar = new AsyncResult(null, msg, null);
                            QcRilHook.notifyRegistrants(ar);
                        }
                    }
                }
            } else {
                Log.w(LOG_TAG, "Received Unknown Intent: action = " + action);
            }
        }
    };

    public void dispose() {
        if (mContext != null) {
            if (mBound == true) {
                Log.v(LOG_TAG,"dispose(): Unbinding service");
                mContext.unbindService(mQcrilMsgTunnelConnection);
                mBound = false;
            }
            Log.v(LOG_TAG,"dispose(): Unregistering receiver");
            mContext.unregisterReceiver(mIntentReceiver);
        }
    }

    /**
     * Creates a ByteBuffer using the byte array passed as a parameter to this
     * method and sets the Byte Order to the native endian format.
     *
     * @param request - byte array used to populate the ByteBuffer
     * @return ByteBuffer that wraps the byte array
     */
    public static ByteBuffer createBufferWithNativeByteOrder(byte[] bytes) {
        ByteBuffer buf = ByteBuffer.wrap(bytes);
        buf.order(ByteOrder.nativeOrder());
        return buf;
    }

    /**
     * Adds the QC RIL Header to the request which consists of the OEM
     * Identifier String, Request ID, Request Size to the ByteBuffer passed to
     * this method
     *
     * @param buf - ByteBuffer to pack the request
     * @param requestId - Type of OEM request
     * @param requestSize - Size of the Request
     * @return None.
     */
    private void addQcRilHookHeader(ByteBuffer buf, int requestId, int requestSize) {
        // Add OEM identifier String
        buf.put(mOemIdentifier.getBytes());

        // Add Request ID
        buf.putInt(requestId);

        // Add Request payload length
        buf.putInt(requestSize);
    }

    /**
     * Main method that shall send the request and wait for response.
     *
     * @param requestId
     * @param request
     * @return Return Code and Response Bytes (if any) in AsyncResult Object
     */
    private AsyncResult sendRilOemHookMsg(int requestId, byte[] request) {
        return sendRilOemHookMsg(requestId, request, DEFAULT_PHONE);
    }

    private AsyncResult sendRilOemHookMsg(int requestId, byte[] request, int phoneId) {
        AsyncResult ar;
        int retVal;
        byte[] response = new byte[RESPONSE_BUFFER_SIZE];

        Log.v(LOG_TAG, "sendRilOemHookMsg: Outgoing Data is " + IccUtils.bytesToHexString(request));

        try {
            retVal = mService.sendOemRilRequestRaw(request, response, phoneId);
            Log.d(LOG_TAG, "sendOemRilRequestRaw returns value = " + retVal);
            if (retVal >= 0) {
                byte[] validResponseBytes = null;
                // retVal contains the actual number of bytes in the response
                if (retVal > 0) {
                    validResponseBytes = new byte[retVal];
                    System.arraycopy(response, 0, validResponseBytes, 0, retVal);
                }
                ar = new AsyncResult(retVal, validResponseBytes, null);
            } else {
                /**
                 * RIL Error was converted to negative value by
                 * sendOemRilRequestRaw method Convert it back to the positive
                 * value and create a exception to return to the caller.
                 */
                byte[] validResponseBytes = new byte[response.length];
                System.arraycopy(response, 0, validResponseBytes, 0, response.length);
                CommandException ex = CommandException.fromRilErrno(-1 * retVal);
                ar = new AsyncResult(request, validResponseBytes, ex);
            }
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "sendOemRilRequestRaw RequestID = " + requestId
                    + " exception, unable to send RIL request from this application", e);

            ar = new AsyncResult(requestId, null, e);
        } catch (NullPointerException e) {
            Log.e(LOG_TAG, "NullPointerException caught at sendOemRilRequestRaw." +
                    "RequestID = " + requestId + ". Return Error");
            ar = new AsyncResult(requestId, null, e);
        }

        return ar;
    }

    /**
     * Main method that shall send the request without waiting for response.
     *
     * @param requestId
     * @param request
     * @param oemHookCb
     * @param phoneId
     */
    private void sendRilOemHookMsgAsync(int requestId, byte[] request, IOemHookCallback oemHookCb,
            int phoneId) throws NullPointerException {
        Log.v(LOG_TAG, "sendRilOemHookMsgAsync: Outgoing Data is "
                + IccUtils.bytesToHexString(request));

        try {
            mService.sendOemRilRequestRawAsync(request, oemHookCb, phoneId);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "sendOemRilRequestRawAsync RequestID = " + requestId
                    + " exception, unable to send RIL request from this application", e);
        } catch (NullPointerException e) {
            Log.e(LOG_TAG, "NullPointerException caught at sendOemRilRequestRawAsync." +
                    "RequestID = " + requestId + ". Throw to the caller");
            throw e;
        }
    }

    //========================================================================
    // FUNCTION: qcRilGetConfig
    //
    // DESCRIPTION: queries RIL for current configuration
    //
    // PARAM: phoneId   phoneId id: 0 or 1 or 2...
    //
    // RETURN: string - the current configuration or null
    //========================================================================
    public String qcRilGetConfig(int phoneId) {
        String result = null;
        byte[] payload = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(payload);

        addQcRilHookHeader(reqBuffer, QCRIL_EVT_HOOK_GET_CONFIG, INT_SIZE);
        reqBuffer.putInt(phoneId);

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_CONFIG, payload);

        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_CONFIG failed w/ "
                           + ar.exception);
            return result;
        }

        if (ar.result == null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_CONFIG failed w/ null result");
            return result;
        }

        // Command executed successfully
        try {
            result = new String((byte[])ar.result, ENCODING);
        } catch (UnsupportedEncodingException e) {
            Log.d(LOG_TAG, "unsupport " + ENCODING);
            return null;
        }
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_CONFIG returned w/ " + result);

        return result;
    }

    //========================================================================
    // FUNCTION: qcRilGetConfig
    //
    // DESCRIPTION: queries RIL for current configuration in phoneId0
    //
    // RETURN: string - the current configuration or null
    //========================================================================
    public String qcRilGetConfig() {
        return qcRilGetConfig(0);
    }

    //========================================================================
    // FUNCTION: qcRilSetConfig
    //
    // DESCRIPTION: send a selected configuration to RIL, with SUBs specified
    //
    // PARAM: file      file name including whole path
    // PARAM: config    config id specified by user
    // PARAM: subMask   sub mask (sub0->1, sub1->2, sub2->4, sub0&sub1->3)
    //
    // RETURN: boolean - success or fail
    //========================================================================
    public boolean qcRilSetConfig(String file, String config, int subMask) {
        if (!config.isEmpty() && config.length() <= MAX_PDC_ID_LEN
                                && !file.isEmpty()) {
            byte[] payload = new byte[mHeaderSize + 3 * BYTE_SIZE + file.length() + config.length()];
            ByteBuffer buf = createBufferWithNativeByteOrder(payload);
            /*
             * Build the payload buffer
             * Format: Header | subMask | file_name | config_id
             */
            addQcRilHookHeader(buf, QCRIL_EVT_HOOK_SET_CONFIG,
                            3 * BYTE_SIZE + file.length() + config.length());
            buf.put((byte)subMask);
            buf.put(file.getBytes());
            buf.put((byte)0);
            try {
                buf.put(config.getBytes(ENCODING));
            } catch (UnsupportedEncodingException e) {
                Log.d(LOG_TAG, "unsupport " + ENCODING);
                return false;
            }
            buf.put((byte)0);

            AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_SET_CONFIG, payload);
            if (ar.exception != null) {
                Log.e(LOG_TAG, "QCRIL_EVT_HOOK_SET_CONFIG failed w/ "
                                + ar.exception);
                return false;
            }
        } else {
            Log.e(LOG_TAG, "set with incorrect config id: " + config);
            return false;
        }

        return true;
    }

    //========================================================================
    // FUNCTION: qcRilSetConfig
    //
    // DESCRIPTION: send a selected configuration to RIL, with config id the
    // same as file name. It is applicable with SUB0 in default.
    //
    // PARAM: file      file name including whole path
    //
    // RETURN: boolean - succes or fail
    //========================================================================
    public boolean qcRilSetConfig(String file) {
        return qcRilSetConfig(file, file, 1);
    }

    //========================================================================
    // FUNCTION: qcRilSetConfig
    //
    // DESCRIPTION: send a selected configuration to RIL, with SUBs specified
    // Config_id is the same as file name
    //
    // PARAM: file      file name including whole path
    // PARAM: subMask   sub mask (sub0->1, sub1->2, sub2->4, sub0&sub1->3)
    //
    // RETURN: boolean - success or fail
    //========================================================================
    public boolean qcRilSetConfig(String file, int subMask) {
        return qcRilSetConfig(file, file, subMask);
    }

    //========================================================================
    // FUNCTION: qcRilGetQcVersionOfFile
    //
    // DESCRIPTION: Get the QC version of a given MBN file
    //
    // PARAM: file      file name including whole path
    //
    // RETURN: null if fails, QC version if succeeds
    //========================================================================
    public byte[] qcRilGetQcVersionOfFile(String file) {
        if (file.isEmpty())
            return null;

        byte[] payload = new byte[mHeaderSize + file.getBytes().length];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE, file.getBytes().length);
        buf.put(file.getBytes());

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE failed w/ "
                                    + ar.exception);
            return null;
        }
        if (ar.result == null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE failed w/ null result");
            return null;
        }

        // Command executed successfully
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE returned w/ " + (byte[])ar.result);
        return (byte[])ar.result;
    }

    //========================================================================
    // FUNCTION: qcRilGetOemVersionOfFile
    //
    // DESCRIPTION: Get the OEM version of a given MBN file
    //
    // PARAM: file      file name including whole path
    //
    // RETURN: null if fails, OEM version if succeeds
    //========================================================================
    public byte[] qcRilGetOemVersionOfFile(String file) {
        if (file.isEmpty())
            return null;

        byte[] payload = new byte[mHeaderSize + file.getBytes().length];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE, file.getBytes().length);
        buf.put(file.getBytes());

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE failed w/ "
                                    + ar.exception);
            return null;
        }
        if (ar.result == null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE failed w/ null result");
            return null;
        }

        // Command executed successfully
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE returned w/ " + (byte[])ar.result);
        return (byte[])ar.result;
    }

    //========================================================================
    // FUNCTION: qcRilGetQcVersionOfID
    //
    // DESCRIPTION: Get the QC version of the given config id
    //
    // PARAM: null
    //
    // RETURN: null if fails, QC version if succeeds
    //========================================================================
    public byte[] qcRilGetQcVersionOfID(String configId) {
        if (configId.isEmpty() || configId.length() > MAX_PDC_ID_LEN) {
            Log.w(LOG_TAG, "invalid config id");
            return null;
        }

        byte[] payload = new byte[mHeaderSize + configId.length()];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID,
                                                configId.length());
        try {
            buf.put(configId.getBytes(ENCODING));
        } catch (UnsupportedEncodingException e) {
            Log.d(LOG_TAG, "unsupport " + ENCODING);
            return null;
        }

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID failed w/ "
                                    + ar.exception);
            return null;
        }
        if (ar.result == null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID failed w/ null result");
            return null;
        }

        // Command executed successfully
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID returned w/ " + (byte[])ar.result);
        return (byte[])ar.result;
    }

    //========================================================================
    // FUNCTION: qcRilGetOemVersionOfID
    //
    // DESCRIPTION: Get the OEM version of the given config_id
    //
    // PARAM: null
    //
    // RETURN: null if fails, OEM version if succeeds
    //========================================================================
    public byte[] qcRilGetOemVersionOfID(String config_id) {
        if (config_id.isEmpty() || config_id.length() > MAX_PDC_ID_LEN) {
            Log.w(LOG_TAG, "invalid config_id");
            return null;
        }

        byte[] payload = new byte[mHeaderSize + config_id.length()];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID,
                                                config_id.length());
        try {
            buf.put(config_id.getBytes(ENCODING));
        } catch (UnsupportedEncodingException e) {
            Log.d(LOG_TAG, "unsupport " + ENCODING);
            return null;
        }

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID failed w/ "
                                    + ar.exception);
            return null;
        }
        if (ar.result == null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID failed w/ null result");
            return null;
        }

        // Command executed successfully
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID returned w/ " + (byte[])ar.result);
        return (byte[])ar.result;
    }

    //========================================================================
    // FUNCTION: qcRilActivateConfig
    //
    // DESCRIPTION: Activate the configuration in the specified sub
    //
    // PARAM: phoneId        phoneId that needs to be activated
    //
    // RETURN: true if activate successfully, otherwise false
    //========================================================================
    public boolean qcRilActivateConfig(int phoneId) {
        byte[] payload = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_ACT_CONFIGS, INT_SIZE);
        buf.putInt(phoneId);

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_ACT_CONFIGS, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_ACT_CONFIGS failed w/ "
                                                + ar.exception);
            return false;
        }
        return true;
    }

    //========================================================================
    // FUNCTION: qcRilValidateConfig
    //
    // DESCRIPTION: Validate the current configuration in modem, compared with
    // the given golden/reference MBN
    //
    // PARAM: configId     config Id of reference MBN
    //        phoneId          phoneId that needs to be verified
    //
    // RETURN: true if send commands to modem successfully, otherwise false
    //========================================================================
    public boolean qcRilValidateConfig(String configId, int phoneId) {
        if (configId.isEmpty() || configId.length() > MAX_PDC_ID_LEN) {
            Log.w(LOG_TAG, "invalid config id");
            return false;
        }
        byte[] payload = new byte[mHeaderSize + 2 * BYTE_SIZE + configId.length()];
        ByteBuffer buf = createBufferWithNativeByteOrder(payload);
        addQcRilHookHeader(buf, QCRIL_EVT_HOOK_VALIDATE_CONFIG,
                            2 * BYTE_SIZE + configId.length());
        buf.put((byte)phoneId);
        try {
            buf.put(configId.getBytes(ENCODING));
        } catch (UnsupportedEncodingException e) {
            Log.d(LOG_TAG, "unsupport " + ENCODING);
            return false;
        }
        buf.put((byte)0);

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_META_INFO, payload);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_VALIDATE_CONFIG failed w/ "
                                                + ar.exception);
            return false;
        }
        return true;
    }

    //========================================================================
    // FUNCTION: qcRilGetAvailableConfigs <device>
    //
    // DESCRIPTION: get a list of available configurations from RIL
    //
    // RETURN: list of strings - available configurations or null
    //========================================================================
    public String[] qcRilGetAvailableConfigs(String device) {

        String[] result = null;

        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS);
        if (ar.exception != null) {
            Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS failed w/ "
                           + ar.exception);
            return null;
        }

        if (ar.result == null) {
            Log.e(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS failed w/ "
                           + "null result");
            return result;
        }

        // Command executed successfully
        Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS raw: "
                       + Arrays.toString((byte[])ar.result));

        // unpack String[] from message data
        try {
            ByteBuffer payload = ByteBuffer.wrap((byte[])ar.result);
            payload.order(ByteOrder.nativeOrder());

            int numStrings = payload.get();
            Log.d(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS success: "
                           + numStrings);

            if (numStrings <= 0) {
                Log.e(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS failed w/"
                           + "invalid payload, numStrings = 0");
                return null;
            }

            // fill in the strings
            result = new String[numStrings];
            for(int i = 0; i < numStrings; i++) {
                byte stringLen = payload.get();
                byte data[] = new byte[stringLen];
                payload.get(data);
                try {
                    result[i] = new String(data, ENCODING);
                } catch (UnsupportedEncodingException e) {
                    Log.d(LOG_TAG, "unsupport " + ENCODING);
                    return null;
                }

                Log.d(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS string "
                               + stringLen + " " + result[i]);
            }
        } catch (BufferUnderflowException e) {
                Log.e(LOG_TAG, "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS failed "
                             + "to parse payload w/ " + e);
        }

        return result;
    }

    //========================================================================
    // FUNCTION: qcRilCleanupConfigs
    //
    // DESCRIPTION: Clean up all configurations loaded in EFS
    //
    // RETURN: boolean - success or fail
    //========================================================================
    public boolean qcRilCleanupConfigs() {
        boolean retval = false;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_DELETE_ALL_CONFIGS);

        if (ar.exception == null) {
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL_EVT_HOOK_DELETE_ALL_CONFIGS failed w/ "
                            + ar.exception);
        }

        return retval;
    }

    //========================================================================
    // FUNCTION: qcRilDeactivateConfigs
    //
    // DESCRIPTION: Deactivate the current config for all SUBs
    //
    // RETURN: boolean - success or fail
    //========================================================================
    public boolean qcRilDeactivateConfigs() {
        boolean retval = false;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_DEACT_CONFIGS);

        if (ar.exception == null) {
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL_EVT_HOOK_DEACT_CONFIGS failed w/ "
                            + ar.exception);
        }

        return retval;
    }

    public boolean qcRilSelectConfig(String config, int subMask) {
        byte[] payload = null;
        if (!config.isEmpty() && config.length() <= MAX_PDC_ID_LEN) {
            try {
                payload = new byte[mHeaderSize + BYTE_SIZE + config.getBytes(ENCODING).length];
                ByteBuffer buf = createBufferWithNativeByteOrder(payload);
                addQcRilHookHeader(buf, QCRIL_EVT_HOOK_SEL_CONFIG,
                                                BYTE_SIZE + config.getBytes(ENCODING).length);
                buf.put((byte)subMask);
                buf.put(config.getBytes(ENCODING));
            } catch (UnsupportedEncodingException e) {
                Log.d(LOG_TAG, "unsupport " + ENCODING);
                return false;
            }

            AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_SEL_CONFIG, payload);
            if (ar.exception != null) {
                Log.e(LOG_TAG, "QCRIL_EVT_HOOK_SEL_CONFIG failed w/ "
                                + ar.exception);
                return false;
            }
        } else {
            Log.e(LOG_TAG, "select with incorrect config id: " + config);
            return false;
        }
        return true;
    }

    public String qcRilGetMetaInfoForConfig(String config) {
        String result = null;
        byte[] payload = null;

        if (!config.isEmpty() && config.length() <= MAX_PDC_ID_LEN) {
            try {
                payload = new byte[mHeaderSize + config.getBytes(ENCODING).length];
                ByteBuffer buf = createBufferWithNativeByteOrder(payload);
                addQcRilHookHeader(buf, QCRIL_EVT_HOOK_GET_META_INFO,
                                                config.getBytes(ENCODING).length);
                buf.put(config.getBytes(ENCODING));
            } catch (UnsupportedEncodingException e) {
                Log.d(LOG_TAG, "unsupport " + ENCODING);
                return null;
            }

            AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_GET_META_INFO, payload);

            if (ar.exception != null) {
                Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_META_INFO failed w/ "
                                + ar.exception);
                return result;
            }

            if (ar.result == null) {
                Log.w(LOG_TAG, "QCRIL_EVT_HOOK_GET_META_INFO failed w/ null result");
                return result;
            }

            // Command executed successfully
            try {
                result = new String((byte[])ar.result, ENCODING);
            } catch (UnsupportedEncodingException e) {
                Log.d(LOG_TAG, "unsupport " + ENCODING);
                return null;
            }
            Log.v(LOG_TAG, "QCRIL_EVT_HOOK_GET_META_INFO returned w/ " + result);
        } else {
            Log.e(LOG_TAG, "get meta info with incorrect config id: " + config);
        }
        return result;
    }

    //========================================================================
    // FUNCTION: qcRilGoDormant
    //
    // DESCRIPTION:
    //
    // RETURN:
    //========================================================================
    public boolean qcRilGoDormant(String interfaceName) {
        boolean retval = false;
        AsyncResult result = sendQcRilHookMsg(QCRILHOOK_GO_DORMANT, interfaceName);

        if (result.exception == null) {
            // Command executed successfully
            retval = true;
        } else {
            Log.w(LOG_TAG, "Go Dormant Command returned Exception: " + result.exception);
        }

        return retval;
    }

    public boolean qcRilSetCdmaSubSrcWithSpc(int cdmaSubscription, String spc) {
        boolean retval = false;

        Log.v(LOG_TAG, "qcRilSetCdmaSubSrcWithSpc: Set Cdma Subscription to "+ cdmaSubscription);

        if (!spc.isEmpty() && spc.length() <= MAX_SPC_LEN)
        {
            byte[] payload = new byte[BYTE_SIZE + spc.length()];
            /*
             *  Build the payload buffer
             *  Format : | cdmaSubscription(1) | SPC(1-6)
             */
            ByteBuffer buf = createBufferWithNativeByteOrder(payload);
            buf.put((byte)cdmaSubscription);
            buf.put(spc.getBytes());

            AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC, payload);

            if (ar.exception == null) {
                if (ar.result != null) {
                    byte[] result = (byte[])ar.result;
                    ByteBuffer byteBuf = ByteBuffer.wrap(result);
                    byte succeed = byteBuf.get();
                    Log.v(LOG_TAG, "QCRIL Set Cdma Subscription Source Command "
                            + ((succeed == 1) ? "Succeed." : "Failed."));
                    if (succeed == 1) {
                        retval = true;
                    } else {
                        retval = false;
                    }
                }
            } else {
                Log.e(LOG_TAG, "QCRIL Set Cdma Subscription Source Command returned Exception: " +
                        ar.exception);
            }
        } else {
            Log.e(LOG_TAG, "QCRIL Set Cdma Subscription Source Command incorrect SPC: " + spc);
        }
        return retval;
    }

    public boolean qcRilSetTuneAway(boolean tuneAway) {
        boolean returnValue = false;
        Log.v(LOG_TAG, "qcRilSetTuneAway: tuneAway Value to be set to " + tuneAway);

        byte payload = (byte)0;
        if (tuneAway) {
           payload = (byte)1;
        }
        Log.v(LOG_TAG, "qcRilSetTuneAway: tuneAway payload " + payload);

        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_SET_TUNEAWAY, payload);

        if (ar.exception == null){
            returnValue = true;
        } else {
            Log.e(LOG_TAG, "qcRilSetTuneAway: Exception " + ar.exception);
        }
        return returnValue;
    }

    public boolean qcRilGetTuneAway() {
        boolean tuneAway = false;

        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_GET_TUNEAWAY);

        if (ar.exception == null) {
            if (ar.result != null) {
                byte[] response = (byte[])ar.result;
                ByteBuffer byteBuffer = ByteBuffer.wrap(response);
                byte tuneAwayValue = byteBuffer.get();
                Log.v(LOG_TAG, "qcRilGetTuneAway: tuneAwayValue " + tuneAwayValue);

                if (tuneAwayValue == 1) {
                    tuneAway = true;
                }
             } else {
                Log.e(LOG_TAG, "qcRilGetTuneAway: Null Response");
             }
        } else{
           Log.e(LOG_TAG, "qcRilGetTuneAway: Exception " + ar.exception);
        }
        return tuneAway;
    }

    public boolean qcRilSetPrioritySubscription(int priorityIndex) {
        boolean returnValue = false;
        Log.v(LOG_TAG, "qcRilSetPrioritySubscription: PrioritySubscription to be set to"
                + priorityIndex);

        byte payload = (byte)priorityIndex;
        Log.v(LOG_TAG, "qcRilSetPrioritySubscription: PrioritySubscription payload " + payload);

        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_SET_PAGING_PRIORITY, payload);

        if (ar.exception == null){
            returnValue = true;
        } else {
            Log.e(LOG_TAG, "qcRilSetPrioritySubscription: Exception " + ar.exception);
        }
        return returnValue;
    }

    public int qcRilGetPrioritySubscription() {
        int subscriptionIndex = 0;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_GET_PAGING_PRIORITY);
        if (ar.exception == null) {
            if (ar.result != null) {
                byte[] response = (byte[]) ar.result;
                subscriptionIndex = ByteBuffer.wrap(response).get();
                Log.v(LOG_TAG, "qcRilGetPrioritySubscription: subscriptionIndex "
                        + subscriptionIndex);
            } else {
                Log.e(LOG_TAG, "qcRilGetPrioritySubscription: Null Response");
            }
        } else {
            Log.e(LOG_TAG, "qcRilGetPrioritySubscription: Exception " + ar.exception);
        }
        return subscriptionIndex;
    }

    public boolean qcRilInformShutDown(int phoneId) {
        Log.d(LOG_TAG, "QCRIL Inform shutdown for phoneId " + phoneId);
        OemHookCallback oemHookCb = new OemHookCallback(null) {
            @Override
            public void onOemHookResponse(byte[] response, int phoneId) throws RemoteException {
                Log.d(LOG_TAG, "QCRIL Inform shutdown DONE!");
            }
        };
        sendQcRilHookMsgAsync(QCRIL_EVT_HOOK_INFORM_SHUTDOWN, null, oemHookCb, phoneId);

        return true;
    }

    public boolean qcRilCdmaAvoidCurNwk() {
        boolean retval = false;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK);

        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL Avoid the current cdma network Command returned Exception: "
                    + ar.exception);
        }
        return retval;
    }

    public boolean qcRilSetFieldTestMode(int phoneId, byte ratType, int enable) {
        boolean retval = false;

        byte[] request = new byte[mHeaderSize + 8];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE, 0);
        reqBuffer.putInt(ratType);
        reqBuffer.putInt(enable);
        Log.d(LOG_TAG, "enable = " + enable + "ratType =" + ratType);
        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE,
                request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL enable engineer mode cmd returned exception: "
                    + ar.exception);
        }
        return retval;
    }

    public boolean qcRilCdmaClearAvoidanceList() {
        boolean retval = false;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST);

        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL Clear the cdma avoidance list Command returned Exception: "
                    + ar.exception);
        }
        return retval;
    }

    public byte[] qcRilCdmaGetAvoidanceList() {
        byte[] retval = null;
        AsyncResult ar = sendQcRilHookMsg(QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST);

        if (ar.exception == null) {
            if (ar.result != null) {
                /*
                 * Avoidance list result buffer
                 * Format : Num of Valid Elements(4) | SID(4) | NID(4) | MNC(4) | MCC(4) | ...
                 */
                byte[] result = (byte[])ar.result;
                if (result.length == AVOIDANCE_BUFF_LEN) {
                    // The size of the avoidance list result buffer
                    // must be 164(4 + 16 * 10)
                    retval = result;
                } else {
                    Log.e(LOG_TAG, "QCRIL Get unexpected cdma avoidance list buffer length: "
                            + result.length);
                }
            } else {
                Log.e(LOG_TAG, "QCRIL Get cdma avoidance list command returned a null result.");
            }

        } else {
            Log.e(LOG_TAG, "QCRIL Get the cdma avoidance list Command returned Exception: "
                    + ar.exception);
        }

        return retval;
    }

    public boolean qcRilPerformIncrManualScan(int phoneId) {
        boolean retval = false;
        byte[] request = new byte[mHeaderSize];

        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN, phoneId);

        AsyncResult ar = sendRilOemHookMsg(QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN,
                request, phoneId);

        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL perform incr manual scan returned exception "
                    + ar.exception);
        }
        return retval;
    }

    public boolean qcrilSetBuiltInPLMNList(byte[] payload, int phoneId) {
        boolean retval = false;
        if (payload == null) {
            Log.e(LOG_TAG, "payload is null");
            return false;
        }

        byte[] request = new byte[mHeaderSize + payload.length];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST,
                payload.length);
        reqBuffer.put(payload);

        AsyncResult ar =  sendRilOemHookMsg(QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST,
                request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL set builtin PLMN list returned exception: "
                    + ar.exception);
        }
        return retval;
    }

    /**
     * FuncName: qcRilSetPreferredNetworkAcqOrder
     *
     * Description: Set preferred network acq order.
     *
     * @param acqOrder       acq order
     * @param phoneId            phoneId
     *
     * @return  true Or false
     */
    public boolean qcRilSetPreferredNetworkAcqOrder(int acqOrder, int phoneId) {
        boolean retval = false;
        int requestId = QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER;
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);


        Log.d(LOG_TAG, "acq order: " + acqOrder);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(acqOrder);

        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL set acq order cmd returned exception: " + ar.exception);
        }

        return retval;
    }

    /**
     * FuncName: qcRilGetPreferredNetworkAcqOrder
     *
     * Description: Get preferred network acq order.
     *
     * @param phoneId            phoneId
     *
     * @return acq order
     */
    public byte qcRilGetPreferredNetworkAcqOrder(int phoneId) {
        byte acq_order = 0;
        int requestId = QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER;
        byte[] request = new byte[mHeaderSize];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);

        if (ar.exception == null){
            if (ar.result != null) {
                byte[] result = (byte[])ar.result;
                ByteBuffer byteBuf = ByteBuffer.wrap(result);
                acq_order = byteBuf.get();
                Log.v(LOG_TAG, "acq order is " + acq_order);
            } else {
                Log.e(LOG_TAG, "no acq order result return");
            }
        } else {
            Log.e(LOG_TAG, "QCRIL set acq order cmd returned exception: " + ar.exception);
        }

        return acq_order;
    }

    /**
     * FuncName: qcRilSetLteTuneaway
     *
     * Description: Enable/Disable LTE tune away on given subscription.
     *
     * @param enable       Enable/Disable LTE tune away
     * @param phoneId            phoneId
     *
     * @return  true Or false
     */

    public boolean qcRilSetLteTuneaway(int enable, int phoneId) {
        boolean retval = false;
        int requestId = QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY;
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);


        Log.d(LOG_TAG, "qcRilSetLteTuneaway enable :" +  enable);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(enable);

        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL set lte tune away returned exception: " + ar.exception);
        }

        return retval;
    }

    /**
     * FuncName: qcRilSendDataEnableStatus
     *
     * Description: Send data enable status
     *
     * @param enable (1-enabled, 0-disabled)
     *
     * @return
     */

    public void qcRilSendDataEnableStatus(int enable, int phoneId) {
        int requestId = QCRIL_EVT_HOOK_SET_IS_DATA_ENABLED;
        OemHookCallback oemHookCb = new OemHookCallback(null) {
            @Override
            public void onOemHookResponse(byte[] response, int phoneId) throws RemoteException {
                Log.d(LOG_TAG, "QCRIL send data enable status DONE!");
            }
        };
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(enable);
        sendRilOemHookMsgAsync(requestId, request, oemHookCb, phoneId);
    }

    /**
     * FuncName: qcRilSendDataRoamingEnableStatus
     *
     * Description: Send data roaming enable status
     *
     * @param enable (1-enabled, 0-disabled)
     *
     * @return
     */

    public void qcRilSendDataRoamingEnableStatus(int enable, int phoneId) {
        int requestId = QCRIL_EVT_HOOK_SET_IS_DATA_ROAMING_ENABLED;
        OemHookCallback oemHookCb = new OemHookCallback(null) {
            @Override
            public void onOemHookResponse(byte[] response, int phoneId) throws RemoteException {
                Log.d(LOG_TAG, "QCRIL send data roaming enable status DONE!");
            }
        };
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(enable);
        sendRilOemHookMsgAsync(requestId, request, oemHookCb, phoneId);
    }

    /**
     * FuncName: qcRilSendApnInfo
     *
     * Description: Send type and apn info
     *
     * Format of oemhook message:
     * int32   apn_type_len;
     * char    apn_type[apn_type_len]; //null terminated string
     * int32   apn_name_len;
     * char    apn_name[apn_name_len]; //null terminated string
     * int32   is_apn_valid; //indicates whether apn is valid or has been removed
     *
     * @param type  type of apn (internet, mms, supl etc)
     * @param apn   apn as mentioned in ApnSettings
     *
     * @return
     */

    public void qcRilSendApnInfo(String type, String apn, int isValid, int phoneId) {
        int requestId = QCRIL_EVT_HOOK_SET_APN_INFO;
        OemHookCallback oemHookCb = new OemHookCallback(null) {
            @Override
            public void onOemHookResponse(byte[] response, int phoneId) throws RemoteException {
                Log.d(LOG_TAG, "QCRIL send apn info DONE!");
            }
        };
        int payloadSize = 3 * INT_SIZE + type.length() + apn.length() + 2 * BYTE_SIZE;
        if (payloadSize > MAX_REQUEST_BUFFER_SIZE) {
            Log.e(LOG_TAG, "APN sent is larger than maximum buffer. Bail out");
            return;
        }
        byte[] request = new byte[mHeaderSize + payloadSize];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);
        addQcRilHookHeader(reqBuffer, requestId, payloadSize);
        reqBuffer.putInt(type.length() + 1);
        reqBuffer.put(type.getBytes());
        reqBuffer.put((byte)0);
        reqBuffer.putInt(apn.length() + 1);
        reqBuffer.put(apn.getBytes());
        reqBuffer.put((byte)0);
        reqBuffer.putInt(isValid);
        sendRilOemHookMsgAsync(requestId, request, oemHookCb, phoneId);
    }

     public boolean qcRilSendDDSInfo(int dds, int phoneId) {
        boolean retval = false;
        int requestId = QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION;
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        Log.d(LOG_TAG, "dds phoneId: " + dds);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(dds);

        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL send dds sub info returned exception: " + ar.exception);
        }
        return retval;
    }

    /**
     * FuncName: qcRilSetPreferredNetworkBandPref
     *
     * Description: Set preferred network band pref.
     *
     * @param bandPref       band pref
     * @param phoneId            phoneId
     *
     * @return  true Or false
     */
    public boolean qcRilSetPreferredNetworkBandPref(int bandPref, int phoneId) {
        boolean retval = false;
        int requestId = QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF;
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);


        Log.d(LOG_TAG, "band pref: " + bandPref);
        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(bandPref);

        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);
        if (ar.exception == null){
            retval = true;
        } else {
            Log.e(LOG_TAG, "QCRIL set band pref cmd returned exception: " + ar.exception);
        }

        return retval;
    }

    /**
     * FuncName: qcRilGetPreferredNetworkBandPref
     *
     * Description: Get preferred network band pref.
     *
     * @param bandType       specify band type(LTE,TDS...)
     * @param phoneId            phoneId
     *
     * @return band pref
     */
    public byte qcRilGetPreferredNetworkBandPref(int bandType, int phoneId) {
        byte band_pref = 0;
        int requestId = QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF;
        byte[] request = new byte[mHeaderSize];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(bandType);

        AsyncResult ar = sendRilOemHookMsg(requestId, request, phoneId);

        if (ar.exception == null){
            if (ar.result != null) {
                byte[] result = (byte[])ar.result;
                ByteBuffer byteBuf = ByteBuffer.wrap(result);
                band_pref = byteBuf.get();
                Log.v(LOG_TAG, "band pref is " + band_pref);
            } else {
                Log.e(LOG_TAG, "no band pref result return");
            }
        } else {
            Log.e(LOG_TAG, "QCRIL get band perf cmd returned exception: " + ar.exception);
        }

        return band_pref;
    }

    public AsyncResult sendQcRilHookMsg(int requestId) {
        byte[] request = new byte[mHeaderSize];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, 0);

        return sendRilOemHookMsg(requestId, request);
    }

    public AsyncResult sendQcRilHookMsg(int requestId, byte payload) {
        return sendQcRilHookMsg(requestId, payload, DEFAULT_PHONE);
    }
    public AsyncResult sendQcRilHookMsg(int requestId, byte payload, int phoneId) {
        byte[] request = new byte[mHeaderSize + BYTE_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, BYTE_SIZE);
        reqBuffer.put(payload);

        return sendRilOemHookMsg(requestId, request, phoneId);
    }

    public AsyncResult sendQcRilHookMsg(int requestId, byte[] payload) {
        return sendQcRilHookMsg(requestId, payload, DEFAULT_PHONE);
    }
    public AsyncResult sendQcRilHookMsg(int requestId, byte[] payload, int phoneId) {
        byte[] request = new byte[mHeaderSize + payload.length];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, payload.length);
        reqBuffer.put(payload);

        return sendRilOemHookMsg(requestId, request, phoneId);
    }

    public AsyncResult sendQcRilHookMsg(int requestId, int payload) {
        byte[] request = new byte[mHeaderSize + INT_SIZE];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, INT_SIZE);
        reqBuffer.putInt(payload);

        return sendRilOemHookMsg(requestId, request);
    }

    public AsyncResult sendQcRilHookMsg(int requestId, String payload) {
        byte[] request = new byte[mHeaderSize + payload.length()];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, payload.length());
        reqBuffer.put(payload.getBytes());

        return sendRilOemHookMsg(requestId, request);
    }

    public void sendQcRilHookMsgAsync(int requestId, byte[] payload, OemHookCallback oemHookCb) {
        sendQcRilHookMsgAsync(requestId, payload, oemHookCb, DEFAULT_PHONE);
    }

    public void sendQcRilHookMsgAsync(int requestId, byte[] payload, OemHookCallback oemHookCb,
            int phoneId) {
        int payloadLength = 0;
        if (payload != null) {
            payloadLength = payload.length;
        }

        byte[] request = new byte[mHeaderSize + payloadLength];
        ByteBuffer reqBuffer = createBufferWithNativeByteOrder(request);

        addQcRilHookHeader(reqBuffer, requestId, payloadLength);
        if (payload != null) {
            reqBuffer.put(payload);
        }
        sendRilOemHookMsgAsync(requestId, request, oemHookCb, phoneId);
    }

    public static void register(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        synchronized (mRegistrants) {
            mRegistrants.add(r);
        }
    }

    public static void unregister(Handler h) {
        synchronized (mRegistrants) {
            mRegistrants.remove(h);
        }
    }

    public void registerForFieldTestData(Handler h, int what, Object obj) {
        // TODO Auto-generated method stub

    }

    public void unregisterForFieldTestData(Handler h) {
        // TODO Auto-generated method stub

    }

    public void registerForExtendedDbmIntl(Handler h, int what, Object obj) {
        // TODO Auto-generated method stub

    }

    public void unregisterForExtendedDbmIntl(Handler h) {
        // TODO Auto-generated method stub

    }

    protected void finalize() {
        Log.v(LOG_TAG,"is destroyed");
    }

    public static void notifyRegistrants(AsyncResult ar) {
        if( mRegistrants != null) {
            mRegistrants.notifyRegistrants(ar);
        } else {
            Log.e(LOG_TAG, "QcRilOemHook notifyRegistrants Failed");
        }
    }

    private ServiceConnection mQcrilMsgTunnelConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = IQcrilMsgTunnel.Stub.asInterface(service);
            if (mService == null) {
                Log.e(LOG_TAG, "QcrilMsgTunnelService Connect Failed (onServiceConnected)");
            } else {
                Log.d(LOG_TAG, "QcrilMsgTunnelService Connected Successfully (onServiceConnected)");
            }
            mBound = true;
            if (mQcrilHookCb != null) {
                Log.d(LOG_TAG, "Calling onQcRilHookReady callback");
                mQcrilHookCb.onQcRilHookReady();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(LOG_TAG, "The connection to the service got disconnected unexpectedly!");
            mService = null;
            mBound = false;
            if (mQcrilHookCb != null) {
                Log.d(LOG_TAG, "Calling onQcRilHookDisconnected callback");
                mQcrilHookCb.onQcRilHookDisconnected();
            }
        }
    };
}
