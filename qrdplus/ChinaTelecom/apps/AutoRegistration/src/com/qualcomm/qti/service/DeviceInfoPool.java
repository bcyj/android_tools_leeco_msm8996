/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.service;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.CellLocation;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.text.TextUtils;
import android.util.Log;

public class DeviceInfoPool {

    public static class DeviceInfoReq {
        String key;
        Message callback;

        public DeviceInfoReq(String key, Message callback) {
            this.key = key;
            this.callback = callback;
        }

    }

    private static final String TAG = DeviceInfoPool.class.getSimpleName();
    private static final String BASE_PARAM_REGVER = "1.0";
    private static final String MAN_PARAM_UETYPE = "1";
    private static final String SIM_TYPE_ICC = "1";
    private static final String SIM_TYPE_UICC = "2";
    private static final String DEFAULT_VALUE = "0";
    private static final boolean DBG = false;
    private static final String RESULT = "result";
    private static final String DEFAULT_HW_VERSION = "PVT2.0";
    private static final String FILENAME_META_VERSION = "/firmware/verinfo/ver_info.txt";

    private TelephonyManager mTelephonyMgr = null;
    private final Context mContext;
    private static DeviceInfoPool mDeviceInfoPool;

    public enum NodeParm {

        REGVER, MEID, MODELSMS, SWVER, SIM1CDMAIMSI,

        UETYPE, SIM1ICCID, SIM2ICCID, SIM1GIMSI, SIM1LTEIMSI, SIM1TYPE, SIM2IMSI, SID, NID, MACID,

        MODELCTA, MANUFACTURE, OSVER, HWVER, PNAME, COLOR, MLPLVER, MSPLVER, CELLID, MMEID,
        ACCESSTYPE, REGDATE,

        NOVALUE;

        public static NodeParm toNodeParam(String str) {
            try {
                if (DBG) {
                    Log.d(TAG, "toNodeParam:" + str);
                }
                return valueOf(str);
            } catch (Exception ex) {
                return NOVALUE;
            }
        }
    }

    private static final int EVENT_GET_DEVICE_INFO = 1;

    private Handler mMainHandler = new Handler(Looper.getMainLooper()) {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_GET_DEVICE_INFO:
                    DeviceInfoReq req = (DeviceInfoReq) msg.obj;
                    Message message = req.callback;
                    if (DBG) {
                        Log.d(TAG, "handle key:" + req.key);
                    }
                    switch (NodeParm.toNodeParam(req.key)) {
                        // Base params
                        case REGVER:
                            response(message, getRegver());
                            break;
                        case MEID:
                            response(message, getMeid());
                            break;
                        case MODELSMS:
                            response(message, getDeviceModel());
                            break;
                        case SWVER:
                            response(message, getSwVersion());
                            break;
                        case SIM1CDMAIMSI:
                            response(message, getSimCdmaIMSI(PhoneConstants.SUB1));
                            break;
                        // Mandatory params
                        case UETYPE:
                            response(message, getUeType());
                            break;
                        case SIM1ICCID:
                            response(message, getSim1Iccid());
                            break;
                        case SIM2ICCID:
                            response(message, getSim2Iccid());
                            break;
                        case SIM1GIMSI:
                            response(message, getSimGIMSI(PhoneConstants.SUB1));
                            break;
                        case SIM1LTEIMSI:
                            response(message, getSimLTEIMSI(PhoneConstants.SUB1));
                            break;
                        case SIM1TYPE:
                            response(message, getSimType(PhoneConstants.SUB1));
                            break;
                        case SIM2IMSI:
                            response(message, getSim2IMSI(PhoneConstants.SUB2));
                            break;
                        case SID:
                            response(message, getSid());
                            break;
                        case NID:
                            response(message, getNid());
                            break;
                        case MACID:
                            response(message, getMacID());
                            break;
                        // Optional params
                        case MODELCTA:
                            response(message, getModelCTA());
                            break;
                        case MANUFACTURE:
                            response(message, getManufacture());
                            break;
                        case OSVER:
                            response(message, getOSVersion());
                            break;
                        case HWVER:
                            response(message, getHWVersion());
                            break;
                        case MLPLVER:
                            mHandler.getMlplVersion(PhoneConstants.SUB1, message);
                            break;
                        case MSPLVER:
                            mHandler.getMsplVersion(PhoneConstants.SUB1, message);
                            break;
                        case ACCESSTYPE:
                            response(message, getAccessType());
                            break;
                        case REGDATE:
                            response(message, getRegDate());
                            break;
                        default:
                            response(message, null);
                    }
            }
        }

    };

    private void init() {
        mTelephonyMgr = TelephonyManager.getDefault();
    }

    private DeviceInfoPool(Context context) {
        mContext = context;
        init();
    }

    public static DeviceInfoPool getInstance(Context context) {
        if (mDeviceInfoPool == null) {
            mDeviceInfoPool = new DeviceInfoPool(context);
        }
        return mDeviceInfoPool;
    }

    public boolean dispatchNodeOperation(String nodeParam, Message message) {
        if (NodeParm.toNodeParam(nodeParam) != NodeParm.NOVALUE) {
            mMainHandler.obtainMessage(EVENT_GET_DEVICE_INFO,
                    new DeviceInfoReq(nodeParam, message))
                    .sendToTarget();
            return true;
        }
        return false;
    }

    private ObtainVersionHandler mHandler = new ObtainVersionHandler();

    private class ObtainVersionHandler extends Handler {
        private static final int MESSAGE_GET_EF_MSPL = 0;
        private static final int MESSAGE_GET_EF_MLPL = 1;
        // MSPL ID is x bytes data
        private static final int NUM_BYTES_MSPL_ID = 5;
        // MLPL ID is 8 bytes data
        private static final int NUM_BYTES_MLPL_ID = 5;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_GET_EF_MSPL:
                    handleGetEFMspl(msg);
                    break;
                case MESSAGE_GET_EF_MLPL:
                    handleGetEFMlpl(msg);
                    break;
            }
        }

        private void handleGetEFMspl(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            String msplVersion = null;
            byte[] data = (byte[]) ar.result;
            if (ar.exception == null) {
                if (data.length > NUM_BYTES_MSPL_ID - 1) {
                    int msplId = ((data[3] & 0xFF) << 8) | (data[4] & 0xFF);
                    msplVersion = String.valueOf(msplId);
                }
            }
            response((Message) ar.userObj, msplVersion);
        }

        private void handleGetEFMlpl(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            String mlplVersion = null;
            byte[] data = (byte[]) ar.result;
            if (ar.exception == null) {
                if (data.length > NUM_BYTES_MLPL_ID - 1) {
                    int mlplId = ((data[3] & 0xFF) << 8) | (data[4] & 0xFF);
                    mlplVersion = String.valueOf(mlplId);
                }
            }
            response((Message) ar.userObj, mlplVersion);
        }

        public boolean getMsplVersion(int slotId, Message message) {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                IccFileHandler fh = controller.getIccFileHandler(slotId,
                        UiccController.APP_FAM_3GPP2);
                if (fh != null) {
                    fh.loadEFTransparent(IccConstants.EF_CSIM_MSPL, NUM_BYTES_MSPL_ID,
                            mHandler.obtainMessage(ObtainVersionHandler.MESSAGE_GET_EF_MSPL,
                                    message));
                    return true;
                }
            }
            response(message, null);
            return false;
        }

        public boolean getMlplVersion(int slotId, Message message) {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                IccFileHandler fh = controller.getIccFileHandler(slotId,
                        UiccController.APP_FAM_3GPP2);
                if (fh != null) {
                    fh.loadEFTransparent(IccConstants.EF_CSIM_MLPL, NUM_BYTES_MLPL_ID,
                            mHandler.obtainMessage(ObtainVersionHandler.MESSAGE_GET_EF_MLPL,
                                    message));
                    return true;
                }
            }
            response(message, null);
            return false;
        }
    }

    private Phone getPhoneInstance() {
        Phone phone = null;
        if (mTelephonyMgr.isMultiSimEnabled()) {
            phone = PhoneFactory.getPhone(PhoneConstants.SUB1);
        } else {
            phone = PhoneFactory.getDefaultPhone();
        }
        return phone;
    }

    public String getRegver() {
        return BASE_PARAM_REGVER;
    }

    public String getDeviceModel() {
        return Build.MODEL;
    }

    public String getMeid() {
        Phone phone = getPhoneInstance();
        if (phone != null)
            return phone.getDeviceId();
        return null;
    }

    public String getSwVersion() {
        String swVersion = SystemProperties.get("ro.build.au_rev", null);
        if (TextUtils.isEmpty(swVersion)) return DEFAULT_VALUE;
        return swVersion;
    }

    public UiccCard getUiccCard(int cardIndex) {
        UiccCard uiccCard = null;
        if (!mTelephonyMgr.isMultiSimEnabled()) {
            uiccCard = UiccController.getInstance().getUiccCard();
        } else {
            uiccCard = UiccController.getInstance().getUiccCard(cardIndex);
        }
        return uiccCard;
    }

    private UiccCardApplication getValidApp(AppType appType, UiccCard uiccCard) {
        UiccCardApplication validApp = null;
        int numApps = uiccCard.getNumApplications();
        for (int i = 0; i < numApps; i++) {
            UiccCardApplication app = uiccCard.getApplicationIndex(i);
            if (app != null && app.getType() != AppType.APPTYPE_UNKNOWN
                    && app.getType() == appType) {
                validApp = app;
                break;
            }
        }
        return validApp;
    }

    public String getSimIMISI(int sub, AppType apptype) {
        String simIMSI = null;
        UiccCardApplication validApp = getValidApp(apptype, getUiccCard(sub));
        if (validApp != null) {
            simIMSI = validApp.getIccRecords().getIMSI();
        }
        return simIMSI;
    }

    public String getSimCdmaIMSI(int sub) {
        String ruimImsi = getSimIMISI(sub, AppType.APPTYPE_RUIM);
        String csimImsi = getSimIMISI(sub, AppType.APPTYPE_CSIM);
        return TextUtils.isEmpty(ruimImsi) ? csimImsi : ruimImsi;
    }

    public String getSimGIMSI(int sub) {
        return getSimIMISI(sub, AppType.APPTYPE_SIM);
    }

    public String getSimLTEIMSI(int sub) {
        return getSimIMISI(sub, AppType.APPTYPE_USIM);
    }

    private String getSim2IMSI(int sub) {
        String mIMSI = null;
        if (mTelephonyMgr.isMultiSimEnabled()) {
            mIMSI = mTelephonyMgr.getSubscriberId(SubscriptionManager.getSubId(sub)[0]);
        } else {
            mIMSI = mTelephonyMgr.getSubscriberId();
        }
        return mIMSI;
    }

    public boolean isUiccCard(UiccCard uiccCard) {
        return uiccCard.isApplicationOnIcc(AppType.APPTYPE_CSIM)
                || uiccCard.isApplicationOnIcc(AppType.APPTYPE_USIM)
                || uiccCard.isApplicationOnIcc(AppType.APPTYPE_ISIM);
    }

    public String getSimType(int sub) {
        UiccCard uiccCard = getUiccCard(sub);
        if (uiccCard == null) {
            return null;
        }
        if (isUiccCard(uiccCard)) {
            return SIM_TYPE_UICC;
        }
        return SIM_TYPE_ICC;
    }

    public String getUeType() {
        return MAN_PARAM_UETYPE;
    }

    public String getSim1Iccid() {
        return getSimIccid(PhoneConstants.SUB1);
    }

    public String getSim2Iccid() {
        return getSimIccid(PhoneConstants.SUB2);
    }

    public String getSimIccid(int subId) {
        String iccid = null;
        if (mTelephonyMgr.isMultiSimEnabled()) {
            iccid = mTelephonyMgr.getSimSerialNumber(subId);
        } else {
            iccid = mTelephonyMgr.getSimSerialNumber();
        }
        return iccid;
    }

    public String getSid() {
        Phone phone = getPhoneInstance();
        if (phone == null) return null;
        String sid = null;
        CellLocation cellLocation = phone.getCellLocation();
        if (cellLocation instanceof CdmaCellLocation) {
            sid = String.valueOf(((CdmaCellLocation) cellLocation).getSystemId());
        }
        return sid;
    }

    public String getNid() {
        Phone phone = getPhoneInstance();
        if (phone == null) return null;
        String nid = null;
        CellLocation cellLocation = phone.getCellLocation();
        if (cellLocation instanceof CdmaCellLocation) {
            nid = String.valueOf(((CdmaCellLocation) cellLocation).getNetworkId());
        }
        return nid;
    }

    public String getMacID() {
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        String macAddress = wifiInfo == null ? null : wifiInfo.getMacAddress();
        return macAddress;
    }

    public String getModelCTA() {
        String modelCTA = Build.MODEL;
        return modelCTA;
    }

    public String getManufacture() {
        String manufacture = Build.MANUFACTURER;
        return manufacture;
    }

    public String getOSVersion() {
        String osVersion = Build.VERSION.RELEASE;
        return osVersion;
    }

    public String getHWVersion() {
        String hwVersion = SystemProperties.get("ro.hw_version", null);
        if (TextUtils.isEmpty(hwVersion)) {
            hwVersion = DEFAULT_HW_VERSION;
        }
        return hwVersion;
    }

    public String getRegDate() {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String time = sdf.format(new java.util.Date());
        return time;
    }

    public String getAccessType() {
        String accessType = null;
        ConnectivityManager connManager = (ConnectivityManager) mContext
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo[] networkInfos = connManager.getAllNetworkInfo();
        for (NetworkInfo networkInfo : networkInfos) {
            if (networkInfo.isConnected()) {
                int networkType = networkInfo.getType();
                if (networkType == ConnectivityManager.TYPE_WIFI
                        || networkType == ConnectivityManager.TYPE_MOBILE) {
                    // 1:network, 2:wifi
                    accessType = String.valueOf(networkType + 1);
                }
            }
        }
        return accessType;
    }

    private static String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename), 256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }

    private void response(Message callback, String result) {
        if (DBG) {
            Log.i(TAG, "response: [callback]=" + callback + " [result]=" + result);
        }
        if (callback == null) {
            return;
        }
        Bundle bundle = new Bundle();
        bundle.putString(RESULT, result);
        if (callback.obj != null && callback.obj instanceof Parcelable) {
            bundle.putParcelable("userobj", (Parcelable) callback.obj);
        }
        callback.obj = bundle;
        if (callback.replyTo != null) {
            try {
                callback.replyTo.send(callback);
            } catch (RemoteException e) {
                Log.w(TAG, "failed to response result", e);
            }
        } else if (callback.getTarget() != null) {
            callback.sendToTarget();
        } else {
            Log.w(TAG, "can't response the result, replyTo and target are all null!");
        }
    }
}
