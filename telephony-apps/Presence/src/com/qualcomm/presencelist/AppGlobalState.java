/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

package com.qualcomm.presencelist;

import java.util.ArrayList;
import java.util.Timer;


import com.qualcomm.presencelist.MainActivity.ContactArrayAdapter;
import com.qualcomm.presencelist.Settings.SettingsMainThreadHandler;
import com.qualcomm.qcrilhook.PresenceOemHook;

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

    private static ListenerHandler mListenerHandler;
    private static Looper mListenerLooper;
    private static int mPendingSubscriptionUriIndex;
    private static ProgressDialog mProgressDialog;
    private static ArrayList<Contact> mContacts;
    private static ContactArrayAdapter<Contact> mAdapter;
    private static ContactInfo mContactInfo;
    private static PresenceOemHook mPresenceOemHook;
    private static Context mMainActivityContext;
    private static SettingsMainThreadHandler mSettingsHandler;

    final static int IMSP_ETAG_EXPIRED  = 0x01;
    final static int IMSP_RAT_CHANGE_LTE   = 0x02;
    final static int IMSP_RAT_CHANGE_EHRPD   = 0x3;
    final static int IMSP_AIRPLANE_MODE   = 0x04;

    final static int NOTIFY_FMT_XML = 0;
    final static int NOTIFY_FMT_STRUCT = 1;

    private static SharedPreferences mMyInfoSettingsHandle;

    private static Timer timerManager;

    public static ArrayList<String> excludedNumberList = new ArrayList();

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

    public static void setPresenceOemHook(PresenceOemHook p) {
        mPresenceOemHook = p;
    }
    public static PresenceOemHook getPresenceOemHook() {
        return mPresenceOemHook;
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
}
