/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Copyright (C) 2009 Qualcomm Technologies, Inc All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location;

import android.os.Bundle;
import android.os.Message;
import android.util.Log;

import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationRequest;
import android.provider.Settings;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;

import android.content.Context;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.ContentValues;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;

import java.util.List;
import java.io.IOException;

import com.qualcomm.location.MonitorInterface.Monitor;

public class Wiper extends Monitor{
    private static final String TAG = "Wiper";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);

    // Handler messages
    private static final int WIPER_INIT_MSG = 0;
    private static final int WIPER_REQUEST_NETWORK_LOCATION_MSG = 1;
    private static final int WIPER_UPDATE_NETWORK_LOCATION_MSG = 2;
    private static final int WIPER_TRIGGER_FREE_WIFI_SCAN_INJECTION_MSG = 3;
    private static final int WIPER_REQUEST_AP_INFO_MSG = 4;
    private static final int WIPER_TRIGGER_WIFI_AP_INFO_INJECTION_MSG = 5;
    private static final int WIPER_UPDATE_PASSIVE_LOCATION_MSG = 6;
    private static final int WIPER_UPDATE_WIFI_AP_ATTACHMENT_STATUS_MSG = 7;
    private static final int WIPER_UPDATE_WIFI_STATUS_MSG = 8;
    private static final int MSG_MAX = 9;

    //wifi request types
    private static final int WIPER_START_PERIODIC_HI_FREQ_FIXES = 0;
    private static final int WIPER_START_PERIODIC_KEEP_WARM = 1;
    private static final int WIPER_STOP_PERIODIC_FIXES = 2;

    private static final int HIGH_FREQ_PERIOD = 3500; //3.5 seconds
    private static final int LOW_FREQ_PERIOD = 5000; //5 seconds

    //feature mapping from izat.conf file
    private static final int ENABLE_ODCPI_MASK = 1;
    private static final int ENABLE_WIFI_SCAN_RESULTS_MASK = 2;
    private static final int ENABLE_SUPL_WIFI_MASK = 4;
    private static final int ENABLE_WIFI_SUPPLICANT_INFO_MASK = 8;

    //wifi supplicant state types
    private static final int WIFI_ACCESS_POINT_ATTACHED = 0;
    private static final int WIFI_ACCESS_POINT_DETACHED = 1;
    private static final int WIFI_ACCESS_POINT_HANDOVER = 2;

    private final Context mContext;
    private LocationManager mLocMgr;
    private WifiManager mWifiMgr;
    private IntentFilter mScanResultIntentFilter;
    private IntentFilter mWifiStatusFilter;
    private IntentFilter mWifiSupplicantStatusFilter;

    private boolean mIsNetworkLocationInSession;
    private boolean mFreeWifiScanEnabled = false;
    private boolean mIsWifiScanInSession = false;
    private int mListenerFlag = 0;

    private List<ScanResult> mResults;
    private boolean mWifiScanCompleted = false;

    private static final int MAX_APS_INFO_LIMIT = 50;
    private static final int MAC_ADDR_LENGTH = 6;
    private static final int SSID_LENGTH = 32;
    private int mRSSI[] = new int[MAX_APS_INFO_LIMIT];
    private int mChannel[] = new int[MAX_APS_INFO_LIMIT];
    private byte mMacAddress[] = new byte[MAX_APS_INFO_LIMIT * MAC_ADDR_LENGTH];
    private int mNumApsUsed;
    private int mApInfoLen;

    private static final String QNP_WIFI_PROVIDER = "GTP-AP-WIFI";
    private static final String QNP_WWAN_PROVIDER = "GTP-AP-WWAN";
    private static final String QNP_ZPP_PROVIDER = "ZPP";

    private byte mAPMacAddress[] = new byte[MAC_ADDR_LENGTH];
    private SupplicantState mLatestSupplicantState;
    private int mCurrentSupplicantState;
    private int mIsAPMacAddressValid;
    private int mIsSSIDValid;
    private int mWifiStatus;
    private char mSSID[];
    private String mWifiAPSSID[] = new String[MAX_APS_INFO_LIMIT];

    public Wiper(MonitorInterface service, int msgIdBase, int listenerFlag) {
        super(service, msgIdBase);

        mContext = mMoniterService.getContext();
        mListenerFlag = listenerFlag;
        if(VERBOSE_DBG)
            Log.v(TAG, "Create Wiper");
        if(VERBOSE_DBG)
            Log.v(TAG, "Listener flag: " + mListenerFlag);

        mLocMgr = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        mWifiMgr = (WifiManager)mContext.getSystemService(mContext.WIFI_SERVICE);
        mScanResultIntentFilter = new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);

        mIsNetworkLocationInSession = false;
        mNumApsUsed = 0;
        mLatestSupplicantState = SupplicantState.DISCONNECTED;

        mContext.registerReceiver(mWifiScanReceiver, mScanResultIntentFilter);

        if((mListenerFlag & ENABLE_WIFI_SCAN_RESULTS_MASK) == ENABLE_WIFI_SCAN_RESULTS_MASK){
            IntentFilter mWifiStatusFilter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
            mContext.registerReceiver(mWifiScanReceiver, mWifiStatusFilter);
        }

        if((mListenerFlag & ENABLE_WIFI_SUPPLICANT_INFO_MASK) == ENABLE_WIFI_SUPPLICANT_INFO_MASK){
            IntentFilter mWifiSupplicantStatusFilter =
                new IntentFilter(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
            mContext.registerReceiver(mWifiScanReceiver, mWifiSupplicantStatusFilter);
        }

        init();

        //add passive provider request
        try {
            LocationRequest request =
                    LocationRequest.createFromDeprecatedProvider(
                            LocationManager.PASSIVE_PROVIDER, 0, 0, false);
            request.setHideFromAppOps(true);
            mLocMgr.requestLocationUpdates(request, mPassiveLocationListener,
                                           mMoniterService.getHandler().getLooper());
        }
        catch(RuntimeException e) {
            if (VERBOSE_DBG)
                Log.e(TAG, "Cannot request for passive location updates");
        }
    }

    /*
     * Called from native code to request network location info
     */
    private void wifiRequestEvent(int type)
    {
        if (VERBOSE_DBG)
            Log.v(TAG, "wifiRequest type: "+ type);
        sendMessage(WIPER_REQUEST_NETWORK_LOCATION_MSG, type, 0, null);
    }

    /*
     * Called from native code to request wifi ap info
     */
    private void wifiApDataRequestEvent()
    {
        if (VERBOSE_DBG)
            Log.v(TAG,"wifiApDataRequestEvent");
        sendMessage( WIPER_REQUEST_AP_INFO_MSG, 0, 0, null);
    }

    /*
     * Checks for position source to identify the horConfidence as well
     * to filter out ZPP fixes
     */
    private int processLocationDataBeforeInjection(Location location)
    {
        Bundle locationBundle = location.getExtras();
        int horConfidence = 68; /*default value */
        if(locationBundle != null) {
            String locationSource = locationBundle.getString("com.qualcomm.location.nlp:source-technology");
            if(locationSource != null) {
                if(VERBOSE_DBG)
                    Log.d(TAG, "locationSource is " + locationSource);
                if(locationSource.equals(QNP_ZPP_PROVIDER)) {
                    if(VERBOSE_DBG)
                        Log.d(TAG, "Remove accuracy so as to filter ZPP fixes");
                    location.removeAccuracy();
                }
            } else {
                if(VERBOSE_DBG)
                    Log.d(TAG, "locationSource is null");
            }
        } else
            if(VERBOSE_DBG)
                Log.d(TAG, "Bundle is empty");

        return horConfidence;
    }

    private void handleNetworkLocationUpdate(Location location) {
        if(VERBOSE_DBG)
            Log.v(TAG, "handleNetworkLocationUpdate lat" +
                  location.getLatitude() + "lon" +
                  location.getLongitude() + "accuracy "
                  + location.getAccuracy());

        int horConfidence = processLocationDataBeforeInjection(location);

        if (location.hasAccuracy() && mWifiScanCompleted)
            native_wiper_send_network_location(1, location.getLatitude(),
                                               location.getLongitude(),
                                               location.getAccuracy(),
                                               horConfidence, 1,
                                               mMacAddress, mRSSI,
                                               mChannel, mNumApsUsed,
                                               mApInfoLen, mWifiAPSSID);
    }

    private void handlePassiveLocationUpdate(Location location) {
        if(VERBOSE_DBG)
            Log.v(TAG, "handlePassiveLocationUpdate lat" +
                  location.getLatitude() + "lon" +
                  location.getLongitude() + "accuracy "
                  + location.getAccuracy());

        int horConfidence = processLocationDataBeforeInjection(location);

        if (location.hasAccuracy())
            native_wiper_send_passive_location(1, location.getLatitude(),
                                               location.getLongitude(),
                                               location.getAccuracy(), horConfidence);
    }

    private void handleFreeWifiScanInjection() {
        if(VERBOSE_DBG)
            Log.v(TAG, "handleFreeWifiScanInjection");
        native_wiper_send_network_location(0, 0, 0, 0, 0, 1, mMacAddress, mRSSI,
                                           mChannel, mNumApsUsed, mApInfoLen, mWifiAPSSID);
    }

    private void handleApInfoInjection() {
        if(VERBOSE_DBG)
            Log.v(TAG, "handleApInfoInjection");
        native_wiper_send_wifi_ap_info(mMacAddress, mRSSI,
                                           mChannel, mApInfoLen);
    }

    private void init() {
        sendMessage(WIPER_INIT_MSG, 0, 0, null);
    }

    private void handleNativeNetworkLocationRequest(int type)
    {
        ContentResolver resolver;
        List<String> providers;
        boolean networkLocProvAvailable = false;
        boolean hasNetworkLocationProvider = false;

        resolver = mContext.getContentResolver();
        providers = mLocMgr.getAllProviders();
        networkLocProvAvailable = (providers.contains(LocationManager.NETWORK_PROVIDER) == true);

        if (networkLocProvAvailable &&
            Settings.Secure.isLocationProviderEnabled(resolver, LocationManager.NETWORK_PROVIDER)) {
            hasNetworkLocationProvider = true;
        } else {
            Log.e(TAG, "LocationManager.NETWORK_PROVIDER not enabled");
        }
        switch(type) {
        case WIPER_START_PERIODIC_HI_FREQ_FIXES:
            if(mIsNetworkLocationInSession == false && hasNetworkLocationProvider == true){
                if (VERBOSE_DBG)
                    Log.v(TAG, "request location updates with high frequency option");
                LocationRequest request =
                        LocationRequest.createFromDeprecatedProvider(
                                LocationManager.NETWORK_PROVIDER, HIGH_FREQ_PERIOD, 0, true);
                request.setHideFromAppOps(true);
                mLocMgr.requestLocationUpdates(request, mNetworkLocationListener,
                                               mMoniterService.getHandler().getLooper());
                mIsNetworkLocationInSession = true;
            }
            break;
        case WIPER_START_PERIODIC_KEEP_WARM:
            if(mIsNetworkLocationInSession == false && hasNetworkLocationProvider == true){
                if (VERBOSE_DBG)
                    Log.v(TAG, "request location updates with low frequency option");
                LocationRequest request =
                        LocationRequest.createFromDeprecatedProvider(
                                LocationManager.NETWORK_PROVIDER, LOW_FREQ_PERIOD, 0, true);
                request.setHideFromAppOps(true);
                mLocMgr.requestLocationUpdates(request, mNetworkLocationListener,
                                               mMoniterService.getHandler().getLooper());
                mIsNetworkLocationInSession = true;
            }
            break;
        case WIPER_STOP_PERIODIC_FIXES:
            if(mIsNetworkLocationInSession = true){
                if (VERBOSE_DBG)
                    Log.v(TAG, "remove updates as stop message recieved");
                mLocMgr.removeUpdates(mNetworkLocationListener);
                mIsNetworkLocationInSession = false;
            }
            break;
        default:
            if(VERBOSE_DBG)
                Log.e(TAG, "Incorrect request sent in: "+type);
        }
    }

    private void handleNativeApInfoRequest()
    {
        if (VERBOSE_DBG)
            Log.v(TAG, "start wifi scan on ap info request from modem");

        if(mIsWifiScanInSession == false)
        {
            mWifiMgr.startScan();
            mIsWifiScanInSession = true;
        }

    }

    private void handleWifiApAttachmentStatusUpdate()
    {
        if (VERBOSE_DBG)
            Log.v(TAG, "update wifi ap attachment status");

        native_wiper_send_wifi_attachment_status(
            mCurrentSupplicantState, mIsAPMacAddressValid, mAPMacAddress, mIsSSIDValid, mSSID);
    }

    private void handleWifiStatusUpdate()
    {
        if (VERBOSE_DBG)
            Log.v(TAG, "update wifi status");

        native_wiper_send_wifi_enabled_status(mWifiStatus);
    }

    static private void logv(String s) {
        if (VERBOSE_DBG)
            Log.v(TAG, s);
    }

    public static native void native_wiper_send_network_location(
        int position_valid, double latitude, double longitude, float accuracy,
        int hor_confidence, int apinfo_valid, byte mac_array[], int rssi_array[],
        int channel_array[], int num_aps_used, int ap_len,
        String ssid_array[]);
    public static native void native_wiper_send_passive_location(
        int position_valid, double latitude, double longitude, float accuracy, int hor_confidence);
    public static native void native_wiper_send_wifi_ap_info(
        byte mac_array[], int rssi_array[], int channel_array[], int ap_len);
    private native void native_wiper_init(int listener_mode);
    private static native void native_wiper_class_init();
    private static native void native_wiper_send_wifi_enabled_status(int status);
    private static native void native_wiper_send_wifi_attachment_status(int status,
                                                                        int ap_mac_valid,
                                                                        byte ap_mac_array[],
                                                                        int ssid_valid,
                                                                        char ssid_array[]);

    @Override
    public int getNumOfMessages() {
        return MSG_MAX;
    }

    @Override
    public void handleMessage(Message msg) {
        int message = msg.what;
        if (VERBOSE_DBG)
            Log.v(TAG, "handleMessage what - " + message);
        switch (message) {
        case WIPER_REQUEST_NETWORK_LOCATION_MSG:
            handleNativeNetworkLocationRequest(msg.arg1);
            break;
        case WIPER_UPDATE_NETWORK_LOCATION_MSG:
            handleNetworkLocationUpdate((Location)msg.obj);
            break;
        case WIPER_INIT_MSG:
            native_wiper_init(mListenerFlag);
            break;
        case WIPER_TRIGGER_FREE_WIFI_SCAN_INJECTION_MSG:
            handleFreeWifiScanInjection();
            break;
        case  WIPER_REQUEST_AP_INFO_MSG:
            handleNativeApInfoRequest();
            break;
        case WIPER_TRIGGER_WIFI_AP_INFO_INJECTION_MSG:
            handleApInfoInjection();
            break;
        case WIPER_UPDATE_PASSIVE_LOCATION_MSG:
            handlePassiveLocationUpdate((Location)msg.obj);
            break;
        case WIPER_UPDATE_WIFI_AP_ATTACHMENT_STATUS_MSG:
            handleWifiApAttachmentStatusUpdate();
            break;
        case WIPER_UPDATE_WIFI_STATUS_MSG:
            handleWifiStatusUpdate();
            break;
        default:
            if (VERBOSE_DBG)
                Log.v(TAG, "unknown message "+message);
        }
    }

    static {
        native_wiper_class_init();
    }

    /*
     * Request for network location info
     */
    private LocationListener mNetworkLocationListener = new LocationListener()
    {
        public void onLocationChanged(Location location) {
            if (VERBOSE_DBG)
                Log.v(TAG, "Active listener onLocationChanged lat" +
                      location.getLatitude()+ "lon" +
                      location.getLongitude() + "accuracy "
                      + location.getAccuracy());

            sendMessage(WIPER_UPDATE_NETWORK_LOCATION_MSG, 0, 0, location);

        }
        public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
            if (VERBOSE_DBG)
                Log.v(TAG, "Status Changed" + arg0);
        }
        public void onProviderEnabled(String arg0) {
            if (VERBOSE_DBG)
                Log.v(TAG, "onProviderEnabled state " + arg0);
        }
        public void onProviderDisabled(String arg0) {
            if (VERBOSE_DBG)
                Log.v(TAG, "onProviderEnabled state " + arg0);
        }
    };


    private LocationListener mPassiveLocationListener = new LocationListener()
    {
        public void onLocationChanged(Location location) {
            if (VERBOSE_DBG)
                Log.v(TAG, "Passive listener onLocationChanged lat" +
                      location.getLatitude()+ "lon" +
                      location.getLongitude() + "accuracy "
                      + location.getAccuracy());
            if (LocationManager.NETWORK_PROVIDER.equals(location.getProvider()) &&
                locationIsFinal(location)) {
                sendMessage(WIPER_UPDATE_PASSIVE_LOCATION_MSG, 0, 0, location);
            }

        }
        public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
            if (VERBOSE_DBG)
                Log.v(TAG, "Status Changed" + arg0);
        }
        public void onProviderEnabled(String arg0) {
            if (VERBOSE_DBG)
                Log.v(TAG, "onProviderEnabled state " + arg0);
        }
        public void onProviderDisabled(String arg0) {
            if (VERBOSE_DBG)
                Log.v(TAG, "onProviderEnabled state " + arg0);
        }
        private boolean locationIsFinal(Location location) {
            Bundle extras = location.getExtras();
            if (extras != null) {
                if (extras.containsKey("com.qualcomm.location.nlp:screen")) {
                    if (VERBOSE_DBG) Log.v(TAG, "Location not final");
                    return false;
                }
            }
            if (VERBOSE_DBG) Log.v(TAG, "Location is final");
            return true;
        }
    };

    /*
     * Receive wifi scan results
     */
    private BroadcastReceiver mWifiScanReceiver = new BroadcastReceiver()
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(intent.getAction())) {
                int i = 0;
                int j = 0;
                int k = 0;
                Integer val = 0;

                mResults = mWifiMgr.getScanResults();
                mWifiScanCompleted = (mResults != null);
                if (mWifiScanCompleted) {
                    mNumApsUsed = mResults.size();
                    if (VERBOSE_DBG)
                        Log.v(TAG,"mNumApsUsed is  "+mNumApsUsed);

                    //clear the existing buffer
                    for(i=0;i<MAX_APS_INFO_LIMIT;i++){
                        for(j=0;j< MAC_ADDR_LENGTH;j++)
                        {
                            mMacAddress[k] = 0;
                            k++;
                        }
                        mRSSI[i] = 0;
                        mChannel[i] = 0;
                    }
                    i = 0;
                    k = 0;

                    for (ScanResult result : mResults)
                    {
                        if (VERBOSE_DBG)
                            Log.v(TAG,"WPS Scanner Result BSSID: " + result.BSSID +
                                  " SSID: " + result.SSID + " RSSI: "+ result.level +
                                  " Channel: "+ result.frequency);

                        if(i<MAX_APS_INFO_LIMIT){
                            String[] mBSSID = result.BSSID.split(":");
                            try {
                                for(j = 0;j<MAC_ADDR_LENGTH; j++,k++){
                                    val = Integer.parseInt(mBSSID[j],16);
                                    mMacAddress[k] = val.byteValue();
                                }
                                mRSSI[i] = result.level;
                                mChannel[i] = result.frequency;
                                mWifiAPSSID[i] = new String(result.SSID);
                                i++;
                            } catch (NumberFormatException e) {
                                    Log.w(TAG, "Unable to parse mac address");
                            }
                            catch (ArrayIndexOutOfBoundsException e) {
                                    Log.w(TAG, "Unable to get ssid");
                            }
                        }
                        else
                            break;
                    }
                    mApInfoLen = i;

                    if((mListenerFlag & ENABLE_WIFI_SCAN_RESULTS_MASK)
                       == ENABLE_WIFI_SCAN_RESULTS_MASK){
                        if (VERBOSE_DBG)
                            Log.v(TAG,"Triggering free wifi scan injection");
                        sendMessage(WIPER_TRIGGER_FREE_WIFI_SCAN_INJECTION_MSG, 0, 0, null);
                    }

                    if(mIsWifiScanInSession == true)
                    {
                        if (VERBOSE_DBG)
                            Log.v(TAG,"Triggering wifi ap info injection");
                        sendMessage(WIPER_TRIGGER_WIFI_AP_INFO_INJECTION_MSG, 0, 0, null);
                        mIsWifiScanInSession = false;
                    }
                }
            }
            else  if (WifiManager.SUPPLICANT_STATE_CHANGED_ACTION.equals(intent.getAction())){
                int i = 0;
                Integer val = 0;

                SupplicantState supplicantState =
                    (SupplicantState)intent.getParcelableExtra(WifiManager.EXTRA_NEW_STATE);
                if (VERBOSE_DBG)
                    Log.v(TAG, "SUPPLICANT_STATE:" + supplicantState.name());

                if (supplicantState == (SupplicantState.COMPLETED)){
                    WifiInfo wifiInfo = mWifiMgr.getConnectionInfo();
                    if (wifiInfo != null)
                    {
                        try {
                            if (VERBOSE_DBG)
                                Log.v(TAG,"Connection info - BSSID - " + wifiInfo.getBSSID()
                                      + " - SSID - " + wifiInfo.getSSID() + "\n");
                            String[] mBSSID =  wifiInfo.getBSSID().split(":");

                            for(i = 0;i<MAC_ADDR_LENGTH; i++){
                                val = Integer.parseInt(mBSSID[i],16);
                                mAPMacAddress[i] = val.byteValue();
                            }

                            mSSID = wifiInfo.getSSID().toCharArray();
                            if((mSSID != null) && (wifiInfo.getSSID().length() <= SSID_LENGTH))
                            {
                                if (VERBOSE_DBG)
                                    Log.v(TAG, "ssid string is valid");
                                mIsSSIDValid = 1;
                            }
                            else
                            {
                                if (VERBOSE_DBG)
                                    Log.v(TAG, "ssid string is invalid");
                                mIsSSIDValid = 0;
                            }

                        } catch (NumberFormatException e) {
                                Log.w(TAG, "Unable to parse mac address");
                        }
                        catch (NullPointerException e) {
                                Log.w(TAG, "Unable to get BSSID/SSID");
                        }

                        if(mLatestSupplicantState == (SupplicantState.DISCONNECTED)){
                            mCurrentSupplicantState = WIFI_ACCESS_POINT_ATTACHED;
                            mIsAPMacAddressValid = 1;
                            sendMessage(WIPER_UPDATE_WIFI_AP_ATTACHMENT_STATUS_MSG, 0, 0, null);
                        }
                        else if(mLatestSupplicantState == (SupplicantState.COMPLETED)){
                            mCurrentSupplicantState = WIFI_ACCESS_POINT_HANDOVER;
                            mIsAPMacAddressValid = 1;
                            sendMessage(WIPER_UPDATE_WIFI_AP_ATTACHMENT_STATUS_MSG, 0, 0, null);
                        }
                        mLatestSupplicantState = SupplicantState.COMPLETED;
                    }
                }
                else if (supplicantState == (SupplicantState.DISCONNECTED) &&
                         mLatestSupplicantState != (SupplicantState.DISCONNECTED)){
                    mCurrentSupplicantState =  WIFI_ACCESS_POINT_DETACHED;
                    mIsAPMacAddressValid = 0;

                    sendMessage(WIPER_UPDATE_WIFI_AP_ATTACHMENT_STATUS_MSG, 0, 0, null);
                    mLatestSupplicantState = SupplicantState.DISCONNECTED;
                }
            }
            else  if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(intent.getAction())) {
                int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0);
                if(WifiManager.WIFI_STATE_DISABLED == wifiState){
                    mWifiStatus = 0;
                }

                if(WifiManager.WIFI_STATE_ENABLED == wifiState){
                    mWifiStatus = 1;
                }
                sendMessage(WIPER_UPDATE_WIFI_STATUS_MSG, 0, 0, null);
            }
        }
    };
}
