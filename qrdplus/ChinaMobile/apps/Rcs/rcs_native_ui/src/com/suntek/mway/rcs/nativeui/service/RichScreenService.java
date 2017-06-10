/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */
package com.suntek.mway.rcs.nativeui.service;
import android.app.Activity;
import android.app.IntentService;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.provider.ContactsContract;
import android.telephony.TelephonyManager;
import android.text.format.Time;
import android.util.Log;
import com.suntek.mway.rcs.nativeui.RcsApiManager;

public class RichScreenService extends IntentService {
    private static int WEEKDAY = 7;
    private static boolean isWifiDisconneted = false;
    private static String DOWNLOAD_EVENT = "111 00 00000";
    // sim card has been switched
    private static String SIM_CARD_HAS_BEEN_REPLACED = "944 90 40000";
    // completed terminal factory reset.
    private static String COMPLETED_RESTORE_FACTORY_SETTINGS = "934 90 30000";

    private static String CARD_IMSI = "CardIMSI";
    private static int DEFAULT_NUMBER_LEN = 11;

    public RichScreenService() {
        // Class name will be the thread name.
        super(RichScreenService.class.getName());

        // Intent should be redelivered if the process gets killed before
        // completing the job.
        setIntentRedelivery(true);
    }

    @Override
    protected void onHandleIntent(final Intent intent) {
        Log.d("RichScreenService:",intent.toString());
        String action = intent.getAction();
        if ("android.net.conn.CONNECTIVITY_CHANGE".equals(action)) {
            ConnectivityManager connManager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
            NetworkInfo mWifi = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (!mWifi.isConnected()) {
                isWifiDisconneted = true;
            } else {
                isWifiDisconneted = false;
            }
            checkNeedUpdateRichScreen();
        } else if ("android.intent.action.BOOT_COMPLETED".equals(action)) {
            SharedPreferences myRichScreenPreference= getSharedPreferences("RichScreenPreference",
                Activity.MODE_PRIVATE);
            String sharePreferenceImsi = myRichScreenPreference.getString(CARD_IMSI,"");
            TelephonyManager mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
            String imsi = mTelephonyMgr.getSubscriberId();
            if("".equals(sharePreferenceImsi)){
                SharedPreferences.Editor editor = myRichScreenPreference.edit();
                editor.putString(CARD_IMSI,imsi);
                editor.commit();                
            } else {
                if(null != imsi && !imsi.equals(sharePreferenceImsi)){
                    try {
                        RcsApiManager.getRichScreenApi().clearRichScrnLocalCache(SIM_CARD_HAS_BEEN_REPLACED);
                    } catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }
            }
        } else if ("android.intent.action.MASTER_CLEAR_NOTIFICATION".equals(action)
                     ||"android.intent.action.MASTER_CLEAR".equals(action)){
                try {
                    RcsApiManager.getRichScreenApi().clearRichScrnLocalCache(COMPLETED_RESTORE_FACTORY_SETTINGS);
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
        }
    }

    private void updateRichScreenForAweek(){
        Log.d("RichScreenService","start update richSreen");
        ContentResolver cr = RichScreenService.this.getContentResolver();
        Cursor cursor = cr.query(ContactsContract.Contacts.CONTENT_URI, null, null, null, null);
        while(cursor.moveToNext()){
            String ContactId = cursor.getString(cursor.getColumnIndex(ContactsContract.Contacts._ID));
            Cursor phone = cr.query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null,
                ContactsContract.CommonDataKinds.Phone.CONTACT_ID + "=" + ContactId, null, null);
            while(phone.moveToNext()){
                if(isWifiDisconneted == true){
                    Log.d("RichScreenService","update interrupted,due to wifi disconneted");
                    return;
                }
                String phoneNumber = phone.getString(phone.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                if(null == phoneNumber)
                    continue;
                phoneNumber = phoneNumber.replaceAll(" ", "");
                phoneNumber = phoneNumber.replaceAll("-", "");
                int len = phoneNumber.length();
                if (len > DEFAULT_NUMBER_LEN){
                    phoneNumber = phoneNumber.substring(len - DEFAULT_NUMBER_LEN, len);
                }
                try {
                    RcsApiManager.getRichScreenApi().downloadRichScrnObj(phoneNumber, DOWNLOAD_EVENT);
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
        cursor.close();
        Log.d("RichScreenService","update compeleted");
        SharedPreferences myRichScreenPreference= getSharedPreferences("RichScreenPreference",
                Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = myRichScreenPreference.edit(); 
        editor.putLong("updateTime", System.currentTimeMillis());
        editor.commit();
    }

    private void checkNeedUpdateRichScreen(){
        SharedPreferences myRichScreenPreference= getSharedPreferences("RichScreenPreference",
            Activity.MODE_PRIVATE);
        long updateTime = myRichScreenPreference.getLong("updateTime",0);
        SharedPreferences.Editor editor = myRichScreenPreference.edit(); 
        if(0 == updateTime){
            editor.putLong("updateTime", System.currentTimeMillis());
            editor.commit();
        } else {
            int days = getDayDifference(new Time(), updateTime, System.currentTimeMillis());
            if(days >= WEEKDAY){
                new Thread() {
                    @Override
                    public void run() {
                        Log.d("RichScreenService","start update richSreen");
                        updateRichScreenForAweek();
                    }
                }.start();
            }
        }
    }

    public int getDayDifference(Time time, long date1, long date2) {
        time.set(date1);
        int startDay = Time.getJulianDay(date1, time.gmtoff);
        time.set(date2);
        int currentDay = Time.getJulianDay(date2, time.gmtoff);
        return Math.abs(currentDay - startDay);
    }
}
