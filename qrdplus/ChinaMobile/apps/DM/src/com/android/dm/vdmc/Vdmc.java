/*
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.dm.vdmc;

import android.os.Message;
import android.util.Log;
import android.content.Context;
import com.android.dm.DmService;
import com.android.dm.DMNativeMethod;
import com.android.dm.DmNetwork;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.net.Uri;
import android.database.Cursor;

import android.provider.Telephony;
import android.content.ContentValues;
import com.android.internal.telephony.TelephonyProperties;

public class Vdmc {
    protected static final String DM_TAG = "DM ==> ";
    private static String TAG = DM_TAG + "Vdmc: ";
    private static Vdmc me = null;
    private static Context _appContext = null;

    private PowerManager mPowerManager;
    private PowerManager.WakeLock mWakeLock;

    public static final int DM_NULL_DIALOG = 0;
    public static final int DM_NIA_INFO_DIALOG = 1;
    public static final int DM_NIA_CONFIRM_DIALOG = 2;
    public static final int DM_ALERT_INFO_DIALOG = 3;
    public static final int DM_ALERT_CONFIRM_DIALOG = 4;
    public static final int DM_ALERT_SINGLE_CHOICE_DIALOG = 5;
    public static final int DM_ALERT_MULTI_CHOICE_DIALOG = 6;
    public static final int DM_CONFIRM_DOWNLOAD_DIALOG = 7;
    public static final int DM_CONFIRM_UPDATE_DIALOG = 8;
    public static final int DM_SIMULATE_UPDATE_DIALOG = 9;
    public static final int DM_PROGRESS_DIALOG = 10;
    public static final int DM_SELFREGIST_DIALOG = 11;
    public static final int DM_DATACONNECT_DIALOG = 12;

    public static boolean isDmSetting = false;
    public static boolean isWAPSetting = false;
    public static boolean isNetSetting = false;
    public static boolean isMmsSetting = false;

    protected VdmcMsgHandler _msgHandler;
    public static String tmpwapapn = null;
    public static String tmpwapport = null;
    public static String tmpwapproxy = null;
    public static String tmpwapuser = null;
    public static String tmpwappwd = null;
    public static String tmpnetapn = null;
    public static String tmpnetport = null;
    public static String tmpnetproxy = null;
    public static String tmpnetuser = null;
    public static String tmpnetpwd = null;
    public static String tmpmms = null;

    public enum DM_START_RESULT_E
    {
        DM_START_NONE,
        DM_START_SUCC,
        DM_START_FAIL,
        DM_START_DONE,
    };

    public enum MMIDM_DM_STATE
    {
        DM_NONE, // dm init state
        DM_START, // dm starte state
        DM_RUN, // dm run state
        DM_CANCEL, // dm cancel state
    };

    public enum SessionType // dm session type
    {
        DM_SESSION_NONE,
        DM_SESSION_USER,
        DM_SESSION_CLIENT,
        DM_SESSION_SERVER,
    };

    private static SessionType _sessionType = SessionType.DM_SESSION_NONE;
    private static String _lastSessionState = "NULL";
    private static int _lastError = 0;

    public static Vdmc getInstance() {
        if (me == null)
        {
            me = new Vdmc();
        }
        return me;
    }

    public static Context getAppContext()
    {
        return DmService.getContext();
    }

    private static class DMThread extends Thread {

        private int dmtype;
        private byte[] dmmessage;
        private int dmmsglen;

        public DMThread(int type, byte[] message, int msglen) {
            dmtype = type;
            dmmessage = message;
            dmmsglen = msglen;
            Log.d(TAG, "DMThread : created !");
        }

        @Override
        public void run() {
            /*
             * for (int i=0; i<dmmessage.length; i++){ Log.d(TAG,
             * "dmmessage["+i+"] = "+dmmessage[i]); }
             */
            Log.d(TAG, "call : JMMIDM_StartVDM !");
            DMNativeMethod.JMMIDM_StartVDM(dmtype, dmmessage, dmmsglen);
        }

    }

    public void startVDM(Context context, SessionType type, byte[] message, String msgOrigin) {

        int result;

        // Log.d(TAG, "startVDM : message = " + message);
        Log.d(TAG, "startVDM : msgOrigin = " + msgOrigin);
        Log.d(TAG, "startVDM : me = " + me);

        // back up session type
        _sessionType = type;
        _msgHandler = new VdmcMsgHandler();
        _appContext = context;

        // Log.d(TAG, "startVDM : _msgHandler = " + _msgHandler);

        if (!DMNativeMethod.JMMIDM_IsDmRun())
        {
            int i = 0;
            int inttype = 0;
            while (message[i] != 0)
                i++;

            if (type == Vdmc.SessionType.DM_SESSION_NONE)
                inttype = 0;
            else if (type == SessionType.DM_SESSION_USER)
                inttype = 1;
            else if (type == SessionType.DM_SESSION_CLIENT)
                inttype = 2;
            else if (type == SessionType.DM_SESSION_SERVER)
                inttype = 3;

            DmNetwork.getInstance().init();

            DMThread dmthd = new DMThread(inttype, message, i);

            mPowerManager = (PowerManager) context.getSystemService(context.POWER_SERVICE);
            mWakeLock = null;
            try {
                mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "dm");
                mWakeLock.acquire();
            } catch (SecurityException e) {
                Log.w(TAG, "No permission to acquire wake lock", e);
                mWakeLock = null;
            }
            Log.d(TAG, "startVDM : acquire wake lock ");
            dmthd.start();

            isDmSetting = false;

            Log.d(TAG, "startVDM : End!");
        }
        else
        {
            // DMNativeMethod.JMMIDM_ExitDM();
            Log.d(TAG, "DM is already run :return!");
        }

    }

    public void stopVDM() {
        Log.d(TAG, "stopVDM : End!");

        _sessionType = SessionType.DM_SESSION_NONE;
        me = null;
        if (isDmSetting)
        {
            isDmSetting = false;
            DmService.getInstance().setAPN(_appContext, DmService.getInstance().getSavedAPN());
            DmService.getInstance().setProxy(_appContext, DmService.getInstance().getSavedProxy());
            DmService.getInstance().setProxyPort(_appContext,
                    DmService.getInstance().getSavedProxyPort());
        }

        int _id = -1;
        int c = 0;
        String selection = "";
        Cursor cursor = null;
        ContentValues values = null;
        // modify for cmwap
        if (isWAPSetting)
        {
            isWAPSetting = false;

            _id = chooseByApn(_appContext, new IccOperatorChooser() {
                public boolean choose(String apnType, Cursor cursor) {
                    if (apnType.contains("default") && apnType.contains("wap")) {
                        return true;
                    } else {
                        return false;
                    }
                }
            });
            Log.d(TAG, "writeGprsCmwapParam     selection:  = " + _id);
            values = new ContentValues();
            if (tmpwapapn != null)
            {
                values.put(Telephony.Carriers.APN, tmpwapapn);
            }
            if (tmpwapport != null)
            {
                values.put(Telephony.Carriers.PORT, tmpwapport);
            }
            if (tmpwapproxy != null)
            {
                values.put(Telephony.Carriers.PROXY, tmpwapproxy);
            }
            values.put(Telephony.Carriers.USER, tmpwapuser);
            values.put(Telephony.Carriers.PASSWORD, tmpwappwd);
            c = values.size() > 0 ? _appContext.getContentResolver().update(
                    Telephony.Carriers.CONTENT_URI,
                    values, Telephony.Carriers._ID + " = " + _id, null) : 0;
            values.clear();
            Log.d(TAG, "writeGprsCmwapParam: " + ", value = " + tmpwapapn + ", update count = " + c);
        }
        // modify end for cmwap

        // add for cmnet
        if (isNetSetting)
        {
            isNetSetting = false;

            _id = chooseByApn(_appContext, new IccOperatorChooser() {
                public boolean choose(String apnType, Cursor cursor) {
                    if (apnType.contains("default") && apnType.contains("net")
                            && !apnType.contains("wap")) {
                        return true;
                    } else {
                        return false;
                    }
                }
            });
            values = new ContentValues();
            if (tmpnetapn != null)
            {
                values.put(Telephony.Carriers.APN, tmpnetapn);
            }
            values.put(Telephony.Carriers.PROXY, tmpnetproxy);
            values.put(Telephony.Carriers.PORT, tmpnetport);
            values.put(Telephony.Carriers.USER, tmpnetuser);
            values.put(Telephony.Carriers.PASSWORD, tmpnetpwd);
            c = values.size() > 0 ? _appContext.getContentResolver().update(
                    Telephony.Carriers.CONTENT_URI,
                    values, Telephony.Carriers._ID + " = " + _id, null) : 0;
            values.clear();
        }
        // add for mms
        if (isMmsSetting)
        {
            isMmsSetting = false;

            _id = chooseByApn(_appContext, new IccOperatorChooser() {
                public boolean choose(String apnType, Cursor cursor) {
                     if (apnType.contains("mms")) {
                         String apn = cursor.getString(cursor
                                 .getColumnIndexOrThrow(Telephony.Carriers.APN));
                         if (apn.contains("wap")) {
                             return true;
                         }
                     }
                     return false;
                }
            });
            values = new ContentValues();
            if (tmpmms != null)
            {
                values.put(Telephony.Carriers.MMSC, tmpmms);
            }
            c = values.size() > 0 ? _appContext.getContentResolver().update(
                    Telephony.Carriers.CONTENT_URI,
                    values, Telephony.Carriers._ID + " = " + _id, null) : 0;
            values.clear();
        }
        // add end
        tmpwapapn = null;
        tmpwapport = null;
        tmpwapproxy = null;
        tmpwapuser = null;
        tmpwappwd = null;
        tmpnetapn = null;
        tmpnetport = null;
        tmpnetproxy = null;
        tmpnetuser = null;
        tmpnetpwd = null;
        tmpmms = null;
        _appContext = null;

        if (mWakeLock != null && mWakeLock.isHeld()) {
            Log.d(TAG, "stopVDM : release wake lock ");
            mWakeLock.release();
        }

    }

    public boolean isVDMRunning()
    {

        return DMNativeMethod.JMMIDM_IsDmRun();
    }

    // get current session type
    public SessionType getSessionType()
    {
        return _sessionType;
    }

    // get last session state
    public static String getLastSessionState()
    {
        return _lastSessionState;
    }

    // get last error code
    public static int getLastError()
    {
        return _lastError;
    }

    // Android event handling
    public void sendMessage(Message msg)
    {
        if (_msgHandler != null)
            _msgHandler.sendMessage(msg);
    }

    private class IccOperatorChooser {
        public boolean choose(String apnType, Cursor cursor) {
            return false;
        }
    }

    public int chooseByApn(Context context, IccOperatorChooser chooser) {
        int id = -1;
        String numeric = android.os.SystemProperties.get(
                TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "");
        String[] operArr = numeric != null ? numeric.split(",") : null;
        //get the registed subId.
        int slotId = DmService.getInstance().getSlotId();
        Log.d(TAG, "registed slotId = " + slotId);
        if (operArr != null && slotId >= 0 && slotId < operArr.length) {
            String oper = operArr[slotId];
            String selection = "numeric=\"" + oper + "\"";
            Cursor cursor = _appContext.getContentResolver().query(
                    Telephony.Carriers.CONTENT_URI,
                    null, selection, null, null);
            if (cursor != null) {
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    String apntype = cursor.getString(cursor
                            .getColumnIndexOrThrow(Telephony.Carriers.TYPE));
                    if (chooser.choose(apntype, cursor)) {
                        id = cursor.getInt(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers._ID));
                        break;
                    }
                    cursor.moveToNext();
                }
                cursor.close();
            }
            if (id != -1) {
                Log.d(TAG, "found success!");
            }
        }
        Log.d(TAG, "chooseByApn id = " + id);
        return id;
    }
}
