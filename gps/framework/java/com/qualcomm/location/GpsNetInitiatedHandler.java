/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/

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

package com.qualcomm.location;

import java.io.UnsupportedEncodingException;
import java.util.Properties;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.location.INetInitiatedListener;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.UserHandle;
import android.util.Log;
import android.util.Slog;
import android.telephony.PhoneStateListener;
import android.content.BroadcastReceiver;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.os.SystemProperties;

import com.android.internal.R;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.TelephonyProperties;

/**
 * A GPS Network-initiated Handler class used by LocationManager.
 *
 * {@hide}
 */
public class GpsNetInitiatedHandler {

    private static final String TAG = "LocSvc_NiH";

    private static final boolean DEBUG = true;
    private static final boolean VERBOSE = false;

    // NI verify activity for bringing up UI (not used yet)
    public static final String ACTION_NI_VERIFY = "android.intent.action.NETWORK_INITIATED_VERIFY";

    // string constants for defining data fields in NI Intent
    public static final String NI_INTENT_KEY_NOTIF_ID = "notif_id";
    public static final String NI_INTENT_KEY_TITLE = "title";
    public static final String NI_INTENT_KEY_MESSAGE = "message";
    public static final String NI_INTENT_KEY_TIMEOUT = "timeout";
    public static final String NI_INTENT_KEY_DEFAULT_RESPONSE = "default_resp";

    // the extra command to send NI response to GpsLocationProvider
    public static final String NI_RESPONSE_EXTRA_CMD = "send_ni_response";

    // the extra command parameter names in the Bundle
    public static final String NI_EXTRA_CMD_NOTIF_ID = "notif_id";
    public static final String NI_EXTRA_CMD_RESPONSE = "response";

    // these need to match GpsNiType constants in gps_ni.h
    public static final int GPS_NI_TYPE_VOICE = 1;
    public static final int GPS_NI_TYPE_UMTS_SUPL = 2;
    public static final int GPS_NI_TYPE_UMTS_CTRL_PLANE = 3;
    public static final int GPS_NI_TYPE_EMERGENCY_SUPL = 4;

    // these need to match GpsUserResponseType constants in gps_ni.h
    public static final int GPS_NI_RESPONSE_ACCEPT = 1;
    public static final int GPS_NI_RESPONSE_DENY = 2;
    public static final int GPS_NI_RESPONSE_NORESP = 3;
    public static final int GPS_NI_RESPONSE_IGNORE = 4;

    // these need to match GpsNiNotifyFlags constants in gps_ni.h
    public static final int GPS_NI_NEED_NOTIFY = 0x0001;
    public static final int GPS_NI_NEED_VERIFY = 0x0002;
    public static final int GPS_NI_PRIVACY_OVERRIDE = 0x0004;

    // these need to match GpsNiEncodingType in gps_ni.h
    public static final int GPS_ENC_NONE = 0;
    public static final int GPS_ENC_SUPL_GSM_DEFAULT = 1;
    public static final int GPS_ENC_SUPL_UTF8 = 2;
    public static final int GPS_ENC_SUPL_UCS2 = 3;
    public static final int GPS_ENC_UNKNOWN = -1;

    private final Context mContext;
    public static GpsNetInitiatedHandler obj;

    // configuration of notificaiton behavior
    private boolean mPlaySounds = false;
    private boolean mPopupImmediately = true;

    // Set to true if string from HAL is encoded as Hex, e.g., "3F0039"
    static private boolean mIsHexInput = true;

    // will be changed form RilInfoMonitor if the
    // device enter or leave emergency moed.
    static public boolean mIsInEmergency = false;

    // read the SUPL_ES form gps.conf
    private static final String PROPERTIES_FILE = "/etc/gps.conf";
    private Properties mProperties;
    private int mIsSuplEsEnabled;

    public static class GpsNiNotification
    {
        public int notificationId;
        public int niType;
        public boolean needNotify;
        public boolean needVerify;
        public boolean privacyOverride;
        public int timeout;
        public int defaultResponse;
        public String requestorId;
        public String text;
        public int requestorIdEncoding;
        public int textEncoding;
        public Bundle extras;
    };

    public static class GpsNiResponse {
        /* User reponse, one of the values in GpsUserResponseType */
        int userResponse;
        /* Optional extra data to pass with the user response */
        Bundle extras;
    };

    // for reading the value of mIsInEmergency
    public boolean readEmergencySUPLStatus()
    {
        synchronized (this) {
            Log.d(TAG, "public boolean readEmergencySUPLStatus():"
                 +"mIsInEmergency is " + mIsInEmergency);
            return mIsInEmergency;
        }
    }

    // for writing the value of mIsInEmergency
    public void updateEmergencySUPLStatus(boolean status)
    {
        synchronized (this) {
            Log.d(TAG, "public boolean updateEmergencySUPLStatus():"
                 +"mIsInEmergency is " + status);
            mIsInEmergency = status;
        }
    }

    // listening for emergency call begins
    public static class OutgoingCallReceiver extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "OutgoingCallReceiver :: onReceive");
            Bundle bundle = intent.getExtras();
            if (bundle == null) {
                Log.e(TAG, "bundle == null");
                return;
            } else {
                Log.d(TAG, "bundle != null");
            }

            String phonenumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
            if (PhoneNumberUtils.isEmergencyNumber(phonenumber)) {
                Log.d(TAG, "PhoneNumberUtils.isEmergencyNumber(phonenumber)");
                if (GpsNetInitiatedHandler.obj != null) {
                    GpsNetInitiatedHandler.obj.updateEmergencySUPLStatus(true);
                }
            }
        }
    }

    /**
     * The notification that is shown when a network-initiated notification
     * (and verification) event is received.
     * <p>
     * This is lazily created, so use {@link #setNINotification()}.
     */
    private Notification mNiNotification;
    private final INetInitiatedListener mNetInitiatedListener;

    public GpsNetInitiatedHandler(Context context, INetInitiatedListener netInitiatedListener) {
        mContext = context;
        mNetInitiatedListener = netInitiatedListener;
        GpsNetInitiatedHandler.obj = this;
        mIsSuplEsEnabled = 0;

        mProperties = new Properties();
        try {
            File file = new File(PROPERTIES_FILE);
            FileInputStream stream = new FileInputStream(file);
            mProperties.load(stream);
            stream.close();

            // read the SUPL_ES form gps.conf
            String isSuplESEnabled = mProperties.getProperty("SUPL_ES");
            if (isSuplESEnabled != null) {
                try {
                    mIsSuplEsEnabled = Integer.parseInt(isSuplESEnabled);
                } catch (NumberFormatException e) {
                    Log.w(TAG, "unable to parse SUPL_ES: " + isSuplESEnabled);
                }
            } else {
                Log.w(TAG, "unable to read SUPL_ES from" + PROPERTIES_FILE);
            }
        } catch (IOException e) {
            Log.w(TAG, "Could not open GPS configuration file " + PROPERTIES_FILE);
        }
    }

    public INetInitiatedListener getNetInitiatedListener() {
        return mNetInitiatedListener;
    }

    // Handles NI events from HAL
    public void handleNiNotification(GpsNiNotification notif)
    {
        if (DEBUG) Log.d(TAG, "in handleNiNotification () :"
                        + " notificationId: " + notif.notificationId
                        + " requestorId: " + notif.requestorId
                        + " text: " + notif.text);

        if (mIsSuplEsEnabled == 0) {
            // SUPL_ES is disabled
            Log.d(TAG, "mIsSuplEsEnabled == 0, calling handleNi(notif)");
            handleNi(notif);
        } else {
            // SUPL_ES is enabled
            Log.d(TAG, "mIsSuplEsEnabled == 1, calling handleNiInEs(notif)");
            handleNiInEs(notif);
        }

        //////////////////////////////////////////////////////////////////////////
        //   A note about timeout
        //   According to the protocol, in the need_notify and need_verify case,
        //   a default response should be sent when time out.
        //
        //   In some GPS hardware, the GPS driver (under HAL) can handle the timeout case
        //   and this class GpsNetInitiatedHandler does not need to do anything.
        //
        //   However, the UI should at least close the dialog when timeout. Further,
        //   for more general handling, timeout response should be added to the Handler here.
        //
    }

    // handle NI form HAL when SUPL_ES is disabled.
    private synchronized void handleNi(GpsNiNotification notif) {
        // legacy behavior
        Log.d(TAG, "entering handleNi(GpsNiNotification notif)");
        if (notif.needNotify) {
            Log.d(TAG, "notif.needNotify is true");
            if (notif.needVerify && mPopupImmediately) {
                // Popup the dialog box now
                Log.d(TAG, "calling openNiDialog(notif)");
                openNiDialog(notif);
            } else {
                // Show the notification
                Log.d(TAG, "calling setNiNotification(notif)");
                setNiNotification(notif);
            }
        }
        // ACCEPT cases: 1. Notify, no verify; 2. no notify, no verify;
        // 3. privacy override.
        if (!notif.needVerify || notif.privacyOverride) {
            Log.d(TAG, "!notif.needVerify || notif.privacyOverride is true");
            try {
                Log.d(TAG, "sending GPS_NI_RESPONSE_ACCEPT");
                mNetInitiatedListener.sendNiResponse(notif.notificationId,
                                                     GPS_NI_RESPONSE_ACCEPT);
            } catch (RemoteException e) {
                Slog.e(TAG, "RemoteException in sendNiResponse");
            }
        }
        Log.d(TAG, "leaving handleNi(GpsNiNotification notif)");
    }

    // handle NI from HAL when the SUPL_ES is enadbled
    private synchronized void handleNiInEs(GpsNiNotification notif) {
        Log.d(TAG, "entering handleNiInEs(GpsNiNotification notif)");

        /* UE is in emergency mode when in emergency call mode or in emergency call back mode*/
        boolean isUEInEmergencyMode = readEmergencySUPLStatus() ||
            Boolean.parseBoolean(SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE));

        Log.d(TAG, "isUEInEmergencyMode is" + isUEInEmergencyMode);
        Log.d(TAG, "readEmergencySUPLStatus() is" + readEmergencySUPLStatus());
        Log.d(TAG, "Boolean.parseBoolean(SystemProperties.get(TelephonyProperties"
             +".PROPERTY_INECM_MODE)) is"
             + Boolean.parseBoolean(SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE)));

        /* Ignore the emergency SUPL INIT when :
               SUPL ES bit is not on but UE is in emmergency mode
           Ignore the emergency SUPL INIT when :
               SUPL ES bit is on but UE is not in emmergency mode
        */
        if ((notif.niType == GPS_NI_TYPE_EMERGENCY_SUPL) != isUEInEmergencyMode) {
            Log.d(TAG, "(notif.niType == GPS_NI_TYPE_EMERGENCY_SUPL) != isUEInEmergencyMode");
            try {
                Log.d(TAG, "ignoring the NI");
                mNetInitiatedListener.sendNiResponse(notif.notificationId,
                                                     GPS_NI_RESPONSE_IGNORE);
            } catch (RemoteException e) {
                Slog.e(TAG, "RemoteException in sendNiResponse");
            }
        } else {
            Log.d(TAG, "(notif.niType == GPS_NI_TYPE_EMERGENCY_SUPL) == isUEInEmergencyMode");
            Log.d(TAG, "calling handleSuplNi(notif)");
            handleNi(notif);
        }
        Log.d(TAG, "leaving handleNiInEs(GpsNiNotification notif)");
    }

    // Sets the NI notification.
    private synchronized void setNiNotification(GpsNiNotification notif) {
        NotificationManager notificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        if (notificationManager == null) {
            return;
        }

        String title = getNotifTitle(notif, mContext);
        String message = getNotifMessage(notif, mContext);

        if (DEBUG) Log.d(TAG, "setNiNotification, notifyId: " + notif.notificationId +
                ", title: " + title +
                ", message: " + message);

        // Construct Notification
        if (mNiNotification == null) {
            mNiNotification = new Notification();
            mNiNotification.icon = com.android.internal.R.drawable.stat_sys_gps_on; /* Change notification icon here */
            mNiNotification.when = 0;
        }

        if (mPlaySounds) {
            mNiNotification.defaults |= Notification.DEFAULT_SOUND;
        } else {
            mNiNotification.defaults &= ~Notification.DEFAULT_SOUND;
        }

        mNiNotification.flags = Notification.FLAG_ONGOING_EVENT | Notification.FLAG_AUTO_CANCEL;
        mNiNotification.tickerText = getNotifTicker(notif, mContext);

        // if not to popup dialog immediately, pending intent will open the dialog
        Intent intent = !mPopupImmediately ? getDlgIntent(notif) : new Intent();
        PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, intent, 0);
        mNiNotification.setLatestEventInfo(mContext, title, message, pi);

        notificationManager.notifyAsUser(null, notif.notificationId, mNiNotification,
                UserHandle.ALL);
    }

    // Opens the notification dialog and waits for user input
    private void openNiDialog(GpsNiNotification notif)
    {
        Intent intent = getDlgIntent(notif);

        if (DEBUG) Log.d(TAG, "openNiDialog, notifyId: " + notif.notificationId +
                ", requestorId: " + notif.requestorId +
                ", text: " + notif.text);

        mContext.startActivity(intent);
    }

    // Construct the intent for bringing up the dialog activity, which shows the
    // notification and takes user input
    private Intent getDlgIntent(GpsNiNotification notif)
    {
        Intent intent = new Intent();
        String title = getDialogTitle(notif, mContext);
        String message = getDialogMessage(notif, mContext);

        // directly bring up the NI activity
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        intent.setClass(mContext, com.qualcomm.location.NetInitiatedActivity.class);

        // put data in the intent
        intent.putExtra(NI_INTENT_KEY_NOTIF_ID, notif.notificationId);
        intent.putExtra(NI_INTENT_KEY_TITLE, title);
        intent.putExtra(NI_INTENT_KEY_MESSAGE, message);
        intent.putExtra(NI_INTENT_KEY_TIMEOUT, notif.timeout);
        intent.putExtra(NI_INTENT_KEY_DEFAULT_RESPONSE, notif.defaultResponse);

        if (DEBUG) Log.d(TAG, "generateIntent, title: " + title + ", message: " + message +
                ", timeout: " + notif.timeout);

        return intent;
    }

    // Converts a string (or Hex string) to a char array
    static byte[] stringToByteArray(String original, boolean isHex)
    {
        int length = isHex ? original.length() / 2 : original.length();
        byte[] output = new byte[length];
        int i;

        if (isHex)
        {
            for (i = 0; i < length; i++)
            {
                output[i] = (byte) Integer.parseInt(original.substring(i*2, i*2+2), 16);
            }
        }
        else {
            for (i = 0; i < length; i++)
            {
                output[i] = (byte) original.charAt(i);
            }
        }

        return output;
    }

    /**
     * Unpacks an byte array containing 7-bit packed characters into a String.
     *
     * @param input a 7-bit packed char array
     * @return the unpacked String
     */
    static String decodeGSMPackedString(byte[] input)
    {
        final char PADDING_CHAR = 0x00;
        int lengthBytes = input.length;
        int lengthSeptets = (lengthBytes * 8) / 7;
        String decoded;

        /* Special case where the last 7 bits in the last byte could hold a valid
         * 7-bit character or a padding character. Drop the last 7-bit character
         * if it is a padding character.
         */
        if (lengthBytes % 7 == 0) {
            if (lengthBytes > 0) {
                if ((input[lengthBytes - 1] >> 1) == PADDING_CHAR) {
                    lengthSeptets = lengthSeptets - 1;
                }
            }
        }

        decoded = GsmAlphabet.gsm7BitPackedToString(input, 0, lengthSeptets);

        // Return "" if decoding of GSM packed string fails
        if (null == decoded) {
            Log.e(TAG, "Decoding of GSM packed string failed");
            decoded = "";
        }

        return decoded;
    }

    static String decodeUTF8String(byte[] input)
    {
        String decoded = "";
        try {
            decoded = new String(input, "UTF-8");
        }
        catch (UnsupportedEncodingException e)
        {
            throw new AssertionError();
        }
        return decoded;
    }

    static String decodeUCS2String(byte[] input)
    {
        String decoded = "";
        try {
            decoded = new String(input, "UTF-16");
        }
        catch (UnsupportedEncodingException e)
        {
            throw new AssertionError();
        }
        return decoded;
    }

    /** Decode NI string
     *
     * @param original   The text string to be decoded
     * @param isHex      Specifies whether the content of the string has been encoded as a Hex string. Encoding
     *                   a string as Hex can allow zeros inside the coded text.
     * @param coding     Specifies the coding scheme of the string, such as GSM, UTF8, UCS2, etc. This coding scheme
     *                      needs to match those used passed to HAL from the native GPS driver. Decoding is done according
     *                   to the <code> coding </code>, after a Hex string is decoded. Generally, if the
     *                   notification strings don't need further decoding, <code> coding </code> encoding can be
     *                   set to -1, and <code> isHex </code> can be false.
     * @return the decoded string
     */
    static private String decodeString(String original, boolean isHex, int coding)
    {
        String decoded = original;
        byte[] input = stringToByteArray(original, isHex);

        switch (coding) {
        case GPS_ENC_NONE:
            decoded = original;
            break;

        case GPS_ENC_SUPL_GSM_DEFAULT:
            decoded = decodeGSMPackedString(input);
            break;

        case GPS_ENC_SUPL_UTF8:
            decoded = decodeUTF8String(input);
            break;

        case GPS_ENC_SUPL_UCS2:
            decoded = decodeUCS2String(input);
            break;

        case GPS_ENC_UNKNOWN:
            decoded = original;
            break;

        default:
            Log.e(TAG, "Unknown encoding " + coding + " for NI text " + original);
            break;
        }
        return decoded;
    }

    // change this to configure notification display
    static private String getNotifTicker(GpsNiNotification notif, Context context)
    {
        String ticker = String.format(context.getString(R.string.gpsNotifTicker),
                decodeString(notif.requestorId, mIsHexInput, notif.requestorIdEncoding),
                decodeString(notif.text, mIsHexInput, notif.textEncoding));
        return ticker;
    }

    // change this to configure notification display
    static private String getNotifTitle(GpsNiNotification notif, Context context)
    {
        String title = String.format(context.getString(R.string.gpsNotifTitle));
        return title;
    }

    // change this to configure notification display
    static private String getNotifMessage(GpsNiNotification notif, Context context)
    {
        String message = String.format(context.getString(R.string.gpsNotifMessage),
                decodeString(notif.requestorId, mIsHexInput, notif.requestorIdEncoding),
                decodeString(notif.text, mIsHexInput, notif.textEncoding));
        return message;
    }

    // change this to configure dialog display (for verification)
    static public String getDialogTitle(GpsNiNotification notif, Context context)
    {
        return getNotifTitle(notif, context);
    }

    // change this to configure dialog display (for verification)
    static private String getDialogMessage(GpsNiNotification notif, Context context)
    {
        return getNotifMessage(notif, context);
    }

}

