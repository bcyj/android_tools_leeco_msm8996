/******************************************************************************
 * @file    ServiceTypeInformer.java
 * @brief   Activity for enabling, disabling, and viewing service types
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 ******************************************************************************/

package com.android.MultiplePdpTest;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.IPhoneStateListener;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import android.os.ServiceManager;
import android.os.RemoteException;

import com.android.MultiplePdpTest.TypeManager.Profile;
import com.android.MultiplePdpTest.ServiceTypeListActivity;

public class ServiceTypeInformer extends Activity {
    /** Activity that displays to users all information
     *  collected regarding a chosen Data Service Type
     */

    String TAG = "MPDP-TEST";
    private static final String POSITION_NUMBER = "com.Telephony.TestPhoneSettings.POS";
    private static final String LIST_APNS = "com.android.MultiplePdpTest.LIST_APNS";
    private Uri CARRIERS_CONTENT = Uri.parse("content://telephony/carriers");

    private int mSelectedApn;
    public static final int[] mProjection = {
        ConnectivityManager.TYPE_MOBILE,
        ConnectivityManager.TYPE_MOBILE,
        ConnectivityManager.TYPE_WIFI,
        ConnectivityManager.TYPE_MOBILE };

    private ConnectivityManager mManager;

    boolean turnOnFlag = false;
    boolean notSet = true;

    public static final String IP_TYPE = "com.android.MultiplePdpTest.IP_VERSION";
    public static final String SERVICE_TYPE = "com.android.MultiplePdpTest.SERVICE_TYPE";

    private static int INVISIBLE = 4;
    private static int GONE = 8;
    private static int VISIBLE = 0;

    // These are the views as set in the XML files //
    private TextView mName;
    private TextView fState;
    private TextView fInter;
    private TextView fIp;
    private TextView mActive;
    private TextView mStat;
    private Button fApn;
    private Button mStart;
    private Button mStop;

    private ITelephony telephonyService;
    public Context context;

    private IntentFilter intentFilter;
    private boolean listenerNotSet = true;

    /** This BroadcastReceiver updates the views on screen to match
     * the information received from the Intent ANY_DATA_STATE
     * This information include IPversion, apn name, state, interface, and type
     * This is for the most part what users expect to see displayed
     */
    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.w(TAG, "Broadcast:" +
                    intent.getAction() + " received " +
                    intent.getStringExtra(PhoneConstants.STATE_CHANGE_REASON_KEY) +
                    " type: " + intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY) +
                    " State: " + intent.getStringExtra(PhoneConstants.STATE_KEY) +
                    " apnName --> " + intent.getStringExtra(PhoneConstants.DATA_APN_KEY));

            ServiceTypeListActivity.typeManager.addOrUpdateType(intent);
            setTextInViews();
        }
    };

    private void connectAllViews() {
        mName = (TextView)findViewById(R.string.type_name);
        mStat = (TextView) findViewById(R.string.enable_disable);
        mActive = (TextView)findViewById(R.string.active_apn);
        fState = (TextView)findViewById(R.string.ipv4_state);
        fIp = (TextView)findViewById(R.string.ipv4_ipaddress);
        fInter = (TextView)findViewById(R.string.ipv4_interface);
        fApn = (Button)findViewById(R.string.ipv4_apns);
        mStart = (Button)findViewById(R.string.start_feature);
        mStop = (Button)findViewById(R.string.stop_feature);
    }

    private ITelephonyRegistry telephonyRegistry;
    PhoneStateListener phoneListener = new PhoneStateListener() {
        @Override
        public void onDataConnectionStateChanged(int state) {
            Log.w(TAG, "THE NEW STATE IS " + Integer.toString(state));
        }

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            Log.w(TAG, "NEW STATE: " + Integer.toString(state) + " NETWORK TYPE: " + Integer.toString(networkType));
        }
    };

    private TelephonyManager teleManager;

    @Override
    public void onCreate(Bundle icicle){
        super.onCreate(icicle);

        if (!ServiceTypeListActivity.sInitialized) {
            Log.d(TAG, "Parent Actvity:ServiceTypeListActivity has not been initialized" +
                    " yet, bailing out.");
            finish();
            return;
        }

        setContentView(R.layout.settings_detail);
        mManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        Intent intent = getIntent();
        mSelectedApn = intent.getIntExtra(ServiceTypeListActivity.SELECTED_SERVICE_TYPE, 0);
        connectAllViews();

        telephonyService = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        telephonyRegistry = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService("telephony.registry"));
        teleManager = (TelephonyManager)getSystemService("phone");
        teleManager.listen(phoneListener, PhoneStateListener.LISTEN_DATA_CONNECTION_STATE);
    }

    @Override
    public void onResume() {
        super.onResume();

        context = getBaseContext();
        intentFilter = new IntentFilter();
        intentFilter.addAction("android.intent.action.ANY_DATA_STATE");
        context.registerReceiver(mReceiver, intentFilter);
        setTextInViews();
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mReceiver);
    }

    private void setTextInViews() {
        if (listenerNotSet) {
            fApn.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    Intent intent = new Intent(LIST_APNS);
                    intent.putExtra(IP_TYPE, "IP");
                    intent.putExtra(SERVICE_TYPE,
                                ServiceTypeListActivity.APN_TYPES[mSelectedApn]);
                    startActivity(intent);
              }
            });

            mStart.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    activateCheck();
                    Log.w(TAG, "Network Feature Start Requested");
                }
            });

            mStop.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    deactivateCheck();
                    Log.w(TAG, "Network Feature Stop Requested");
                }
            });

            if (mSelectedApn ==
                    ServiceTypeListActivity.LIST_ENTRIES.indexOf(PhoneConstants.APN_TYPE_DEFAULT)) {
                mStart.setVisibility(INVISIBLE);
                mStart.setClickable(false);
                mStop.setVisibility(INVISIBLE);
                mStop.setClickable(false);
                mStat.setVisibility(GONE);
            }

            listenerNotSet = false;
        }
       mName.setText(ServiceTypeListActivity.LIST_ENTRIES.get(mSelectedApn));

       showChanges();
    }

    private String checkNull(String value) {
        /** To be used only so that
         * values will not appear to users as "NULL"
         */
        if (value != null) {
            return value;
        }
        return " ";
    }

    private void showChanges() {
        Profile mProfile = ServiceTypeListActivity.typeManager.getProfileFromType(mSelectedApn);
        if (mProfile == null) {
            showDefault();
            return;
        }

        fState.setText(checkNull(mProfile.getState()));
        fInter.setText(checkNull(mProfile.getInterFace()));
        mActive.setText(checkNull(mProfile.getName()));
    }

    private void showDefault() {
        fState.setText("Not Available");
        fInter.setText("Not Available");
        mActive.setText("Not Available");
    }

    private boolean activateCheck() {
        int result = mManager.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
            ServiceTypeListActivity.APN_FEATURE_TYPES[mSelectedApn]);

        if (result == PhoneConstants.APN_ALREADY_ACTIVE) {
            Log.w(TAG, "startUsingNetworkFeature returned " + Integer.toString(result) + ", ALREADY_ACTIVE");
            mStat.setText("Feature already active");
            return true;
        } else if (result == PhoneConstants.APN_REQUEST_STARTED) {
            Log.w(TAG, "startUsingNetworkFeature returned " + Integer.toString(result) + ", APN_REQUEST_STARTED");
            mStat.setText("Feature Request Started");
            return true;
        } else if (result == PhoneConstants.APN_TYPE_NOT_AVAILABLE) {
            Log.w(TAG, "startUsingNetworkFeature returned " + Integer.toString(result) + ", APN_TYPE_NOT_AVAILABLE");
            mStat.setText("Apn Type Not Available");
            return false;
        } else if (result == -1) {
            Log.w(TAG, "startUsingNetwokFeature returned " + Integer.toString(result) + ", FAILURE");
            mStat.setText("Request Failed");
        }
        return true;
    }

    private boolean deactivateCheck() {
        int result = mManager.stopUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
                ServiceTypeListActivity.APN_FEATURE_TYPES[mSelectedApn]);
        if (result != -1) {
            mStat.setText("Press Start to Enable Network Feature");
            return true;
        }
        mStat.setText("Disable Request Failed");
        return false;
    }
}
