/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.util.ArrayList;
import java.util.Timer;


import com.qualcomm.qti.presence.IQPresService;
import com.qualcomm.qti.presenceapp.MainActivity.ContactArrayAdapter;
import com.qualcomm.qti.presenceapp.Settings.SettingsMainThreadHandler;
import com.qualcomm.qti.rcsservice.IQRCSService;
import com.qualcomm.qti.rcsservice.QRCSInt;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Looper;

public class AppGlobalState {

    public static final String IMS_PRESENCE_MY_INFO = "ImsPresencePrefMyInfo";
    public static final String IMS_PRESENCE_PAR = "ImsPresencePrefPar";
    public static final String IMS_PRESENCE_SETTINGS = "ImsPresencePrefSettings";
    public static final String IMS_PRESENCE_LIVE_LOGGING = "LiveLoggingActivity.txt";
    public static final String IMS_PRESENCE_NOTIFY_XML_FILE = "notify.xml";

    private static int operatorMode = 0;
    public static final int VZW_MODE = 0;
    public static final int ATT_MODE = 5;

    private static ListenerHandler mListenerHandler;
    private static Looper mListenerLooper;
    private static int mPendingSubscriptionUriIndex;
    private static ProgressDialog mProgressDialog;
    private static ArrayList<Contact> mContacts;
    private static ContactArrayAdapter<Contact> mAdapter;
    private static ContactInfo mContactInfo;
    private static IQRCSService iqRcsService;
    private static IQPresService iqPresService;
    private static int presenceSerrviceHandle = 0;
    private static QRCSInt presenceListenerHandle = new QRCSInt();
    private static Context mMainActivityContext;
    private static SettingsMainThreadHandler mSettingsHandler;
    private static String imsEnablerState;
    private static int pUserData = 0;

    public static boolean isDataSettingNvSame = false;

    final static int IMSP_ETAG_EXPIRED  = 0x01;
    final static int IMSP_RAT_CHANGE_LTE   = 0x02;
    final static int IMSP_RAT_CHANGE_EHRPD   = 0x3;
    final static int IMSP_AIRPLANE_MODE   = 0x04;

    final static int NOTIFY_FMT_XML = 0;
    final static int NOTIFY_FMT_STRUCT = 1;

    private static SharedPreferences mMyInfoSettingsHandle;

    private static Timer timerManager;

    private static int qrcsImsSettingsclienthandle  = 0;

    public static ArrayList<RequestInfo> requestinfo = new ArrayList<RequestInfo>();

    public static ArrayList<String> excludedNumberList = new ArrayList<String>();

    static public ListenerHandler getListenerHandler() {
        return mListenerHandler;
    }

    static public void setListenerHandler(ListenerHandler listenerHandler) {
        mListenerHandler = listenerHandler;
    }

    public static void setListenerLooper(Looper listenerLooper) {
        mListenerLooper = listenerLooper;

    }

    public static  Looper getListenerLooper() {
        return mListenerLooper;

    }

    public static void setPendingSubscriptionUri(int contactIndex) {
        mPendingSubscriptionUriIndex = contactIndex;
    }

    public static int getPendingSubscriptionUri() {
        return mPendingSubscriptionUriIndex;
    }

    public static void setProgressDialog(ProgressDialog p) {
        mProgressDialog = p;
    }

    public static ProgressDialog getProgressDialog() {
        return mProgressDialog;
    }

    public static void setContacts(ArrayList<Contact> contacts) {
        mContacts = contacts;
    }

    public static ArrayList<Contact> getContacts() {
        return mContacts;
    }

    public static void setMainListAdapter(ContactArrayAdapter<Contact> adapter) {
        mAdapter = adapter;
    }

    public static ContactArrayAdapter<Contact> getMainListAdapter() {
        return mAdapter;
    }

    public static void setContactInfo(ContactInfo contactInfo) {
        mContactInfo = contactInfo;
    }

    public static ContactInfo getContactInfo( ) {
        return mContactInfo;
    }

    public static void setMyInfoSettingHandle(SharedPreferences setting) {
        mMyInfoSettingsHandle = setting;
    }
    public static SharedPreferences  getMyInfoSettingHandle( ) {
        return mMyInfoSettingsHandle;
    }

    public static void setPresenceService(IQPresService p) {
        iqPresService = p;
    }
    public static IQPresService getPresenceService() {
        return iqPresService;
    }

    public static void setTimerManager(Timer tm) {
        timerManager = tm;
    }
    public static Timer getTimerManager() {
        return timerManager;
    }

    public static void setMainActivityContext(Context mContext) {
        mMainActivityContext = mContext;
    }
    public static Context getMainActivityContext() {
        return mMainActivityContext;
    }

    public static void setSettingsHandler(
            SettingsMainThreadHandler settingsHandler) {
        mSettingsHandler = settingsHandler;
    }

    public static SettingsMainThreadHandler getSettingsHandler() {
        return mSettingsHandler;
    }

    public static IQRCSService getIqRcsService() {
        return iqRcsService;
    }

    public static void setIqRcsService(IQRCSService iqRcsService) {
        AppGlobalState.iqRcsService = iqRcsService;
    }

    public static int getPresenceSerrviceHandle() {
        return presenceSerrviceHandle;
    }

    public static void setPresenceSerrviceHandle(int presenceSerrviceHandle) {
        AppGlobalState.presenceSerrviceHandle = presenceSerrviceHandle;
    }

    public static String getImsEnablerState() {
        return imsEnablerState;
    }

    public static void setImsEnablerState(String imsEnablerState) {
        AppGlobalState.imsEnablerState = imsEnablerState;
    }

    public static QRCSInt getPresenceListenerHandle() {
        return presenceListenerHandle;
    }

    public static void setPresenceListenerHandle(QRCSInt presenceListenerHandle) {
        AppGlobalState.presenceListenerHandle = presenceListenerHandle;
    }

    public static int getpUserData() {
        return pUserData++;
    }

    public static void setpUserData(int pUserData) {
        AppGlobalState.pUserData = pUserData;
    }

    public static int getpUserDataValue() {
        return pUserData;
    }

    public static int getQrcsImsSettingsclienthandle() {
        return qrcsImsSettingsclienthandle;
    }

    public static void setQrcsImsSettingsclienthandle(
            int qrcsImsSettingsclienthandle) {
        AppGlobalState.qrcsImsSettingsclienthandle = qrcsImsSettingsclienthandle;
    }

    public static int getOperatorMode() {
        return operatorMode;
    }

    public static void setOperatorMode(int operatorMode) {
        AppGlobalState.operatorMode = operatorMode;
    }
}

class RequestInfo
{
    public int userData = 0;
    public int requestID = 0;
    public String [] URI;
}
