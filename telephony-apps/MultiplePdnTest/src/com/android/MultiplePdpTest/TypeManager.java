/******************************************************************************
 * @file    TypeManager.java
 * @brief   Special class for storing PDP information from intents
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 ******************************************************************************/

package com.android.MultiplePdpTest;

import java.util.ArrayList;
import java.util.HashMap;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import android.util.Log;
import android.content.Intent;
import android.net.LinkProperties;
import android.content.Context;

public class TypeManager{
    /** This class is used to maintain all the statuses of the various
     * Data Service Types in a HashMap of Profiles
     * Each profile corresponds to a specific type, ipversion data connection
     * It has all the attributes of a data connection, i.e. state, interface, etc..
     */

    private String TAG = "MPDP-TEST";
    private HashMap<String, Profile> dataProfileList;
    // List of APNs received in the broadcasted data connection updates
    private ArrayList<String> apnsRecvd;
    private boolean DBG = true;

    public class Profile {
        private String apnType;
        private String apnState;
        private String inFace;
        private String ipAdd;
        private String apnName;

        public Profile(String apnType, String apnState, String inFace,
                String ipAdd, String apnName) {
            this.apnType = apnType;
            this.apnState = apnState;
            this.inFace = inFace;
            this.ipAdd = ipAdd;
            this.apnName = apnName;
        }

        public boolean hasSameConnectionState(String otherState) {
            return apnState.equals(otherState);
        }

        public boolean isDisconnected() {
            return apnState.equals("DISCONNECTED");
        }

        public boolean isConnected() {
            return apnState.equals("CONNECTED");
        }

        public boolean isConnecting() {
            return apnState.equals("CONNECTING");
        }

        public String getName() {
            return apnName;
        }

        public String getType() {
            return apnType;
        }

        public String getState() {
            return apnState;
        }

        public String getInterFace() {
            return inFace;
        }

        public String getIpAddress() {
            return ipAdd;
        }

        public boolean sameTypeAs(Profile other) {
            return apnType.equals(other.getType());
        }
    }

    public TypeManager(Context context) {
        dataProfileList = new HashMap<String, Profile>();
        apnsRecvd = new ArrayList<String>();
    }

    public void addOrUpdateType(Intent intent) {
        String apnType = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
        String state = intent.getStringExtra(PhoneConstants.STATE_KEY);
        String apnName = intent.getStringExtra(PhoneConstants.DATA_APN_KEY);
        String iface = intent.getStringExtra(PhoneConstants.DATA_IFACE_NAME_KEY);
        LinkProperties lp = intent.getParcelableExtra(PhoneConstants.DATA_LINK_PROPERTIES_KEY);

        String ipAddress = null;
        if (lp != null) {
            ipAddress = lp.getAddresses().toString();
            Log.w(TAG, "LinkProperties:" + lp.toString());
        }

        if(apnType == null) {
            Log.w(TAG, "apnType is null!");
            return;
        }

        Profile profile = new Profile (apnType, state, iface, ipAddress, apnName);
        Log.w(TAG, "Storing profile for apn:" + apnType);
        if (!apnsRecvd.contains(apnType)) {
            apnsRecvd.add(apnType);
        }
        dataProfileList.put(apnType, profile);
    }

    public Profile getProfileFromType(int typeIndex) {
        if (DBG) {
            showDetailedProfileLogs();
        }
        String correctProfile = null;
        for (String key : apnsRecvd) {
            if (key.contains(ServiceTypeListActivity.APN_TYPES[typeIndex])) {
                correctProfile = key;
                break;
            }
        }
        if (correctProfile == null) {
            return null;
        }
        return (Profile) dataProfileList.get(correctProfile);
    }

    private void showDetailedProfileLogs() {
        Log.w(TAG, "************************************************");
        Log.w(TAG, "Name | Type | State | IPAddress");
        Log.w(TAG, "--------------------------");

        for (String key : apnsRecvd) {
            Profile profile = (Profile) dataProfileList.get(key);
            if(profile == null) continue;
            Log.w(TAG, profile.getName() + " | " +
                    profile.getType() + " | " + profile.getState() +
                " | " + profile.getIpAddress());
            Log.w(TAG, "--------------------------");
        }
    }
}
