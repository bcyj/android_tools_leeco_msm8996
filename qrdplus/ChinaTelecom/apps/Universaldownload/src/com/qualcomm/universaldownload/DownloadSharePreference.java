/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class DownloadSharePreference {
    private static final String TAG = "DownloadSharePreference";

    private static final String SHARED_PREFS = "download_prefs";
    private static final String AUTO_UPDATE = "auto_update";
    private static final String SERVER_ADDR = "server_address";
    private static final String LAST_UPDATE_TIME = "last_update_time";
    private static final String DATA_COUNT = "data_count";
    private static final String CAPABILITY_NAME = "Capability";
    private static final String DEFAULT_NONAME = "";
    private static final String VERSION = "Version";
    private static final String CHECKED = "Checked";
    private static final String RESPONSE_ADDR = "Response_address";
    private static final String DEFAULT_ADDR = "https://roam.radiosky.com.cn/cdma/ud/index";
    private static final String PASSIVE_ADDRESS = "Passive_address";
    private static final String AUTO_UPDATE_TIME = "auto_update_time";
    private static DownloadSharePreference instance;
    private final Context mContext;
    private boolean mIsAutoUpdate;
    private String mServerAddress;
    private String mLastUpdateTime;
    private List<UpdateItem> mUpdateItems;
    private String mResponseAddr;
    private autoUpdateChangedListener mAutoupdateListenr;
    private String mPassiveAddr;
    private long mAutoUpdateTime;

    public void setPassiveAddress(String url) {
        mPassiveAddr = url;
    }

    public String getPassiveAddress() {
        return mPassiveAddr;
    }

    public void setAlarmTime(long updateTime) {
        mAutoUpdateTime = updateTime;
    }

    public long getAlarmTime() {
        return mAutoUpdateTime;
    }

    public interface autoUpdateChangedListener {
        void onAutoUpdateChanged(boolean autoUpdate);
    }
    private DownloadSharePreference(Context context) {
        mContext = context;
        loadConfig();
    }

    public static DownloadSharePreference Instance(Context context) {
        if(instance == null) {
            instance = new DownloadSharePreference(context);
        }
        return instance;
    }

    private void loadConfig() {
        Log.d(TAG, "Load preference");

        if(mContext == null) {
            return;
        }
        SharedPreferences sp = mContext.getSharedPreferences(SHARED_PREFS, Context.MODE_PRIVATE);
        mIsAutoUpdate = sp.getBoolean(AUTO_UPDATE, true);
        mServerAddress = sp.getString(SERVER_ADDR, DEFAULT_ADDR);
        mResponseAddr = sp.getString(RESPONSE_ADDR, DEFAULT_NONAME);
        mLastUpdateTime = sp.getString(LAST_UPDATE_TIME, DEFAULT_NONAME);
        if(mServerAddress.isEmpty()) {
            mServerAddress = DEFAULT_ADDR;
        }

        int dataCount = sp.getInt(DATA_COUNT, 0);
        mUpdateItems = new ArrayList<UpdateItem>();

        for(int i = 0; i < dataCount; i++) {
            String capability_name = sp.getString(CAPABILITY_NAME + i, DEFAULT_NONAME);
            String version = sp.getString(VERSION + i, DEFAULT_NONAME);
            boolean checked = sp.getBoolean(CHECKED + i, true);
            UpdateItem ui = new UpdateItem(capability_name, version);
            ui.check(checked);
            mUpdateItems.add(ui);
        }
        if(mUpdateItems.size() == 0) {
            String[] items = mContext.getResources().getStringArray(R.array.pref_update_items);
            for(String item :items) {
                UpdateItem uItem = new UpdateItem(item, "");
                mUpdateItems.add(uItem);
            }
        }
        mPassiveAddr = sp.getString(PASSIVE_ADDRESS, null);
        mAutoUpdateTime = sp.getLong(AUTO_UPDATE_TIME, 0);
    }

    public void save() {
        if(mContext == null) {
            return;
        }
        Log.d(TAG, "save preference");

        SharedPreferences sp = mContext.getSharedPreferences(SHARED_PREFS, mContext.MODE_PRIVATE);
        SharedPreferences.Editor edit = sp.edit();
        edit.clear();
        edit.putBoolean(AUTO_UPDATE, mIsAutoUpdate);
        edit.putString(SERVER_ADDR, mServerAddress);
        edit.putString(RESPONSE_ADDR, mResponseAddr);
        edit.putString(LAST_UPDATE_TIME, mLastUpdateTime);

        int data_count = mUpdateItems.size();
        edit.putInt(DATA_COUNT, data_count);
        for (int i = 0; i < data_count; i++) {
            edit.putString(CAPABILITY_NAME + i, mUpdateItems.get(i).getCapabilityName());
            edit.putString(VERSION + i, mUpdateItems.get(i).getVersion());
            edit.putBoolean(CHECKED + i, mUpdateItems.get(i).isChecked());
        }
        if(mPassiveAddr != null) {
            edit.putString(PASSIVE_ADDRESS, mPassiveAddr);
        }
        edit.putLong(AUTO_UPDATE_TIME, mAutoUpdateTime);
        edit.commit();
    }

    public void setServerAddress(String addr) {
        mServerAddress = addr;
    }

    public void setAutoUpdate(boolean autoUpdate) {
        mIsAutoUpdate = autoUpdate;
        if(mAutoupdateListenr != null) {
            mAutoupdateListenr.onAutoUpdateChanged(autoUpdate);
        }
    }

    public void updateLastTime(String time) {
        mLastUpdateTime = time;
    }

    public void changeCapability(String capabilityName, String version) {
        int index = containsItem(capabilityName);
        if(index != -1) {
            mUpdateItems.get(index).setVersion(version);
            SystemProperties.set("persist.env.phone.pnlversion", version);
        } else {
            mUpdateItems.add(new UpdateItem(capabilityName, version));
        }
    }

    private int containsItem(String capabilityName) {
        int index = 0;
        for(UpdateItem item : mUpdateItems) {
            if(item.getCapabilityName().equals(capabilityName)) {
                return index;
            }
            index++;
        }
        return -1;
    }

    public void checkCapability(String capabilityName, boolean checked) {
        if(mUpdateItems.size() != 0) {
            int index = containsItem(capabilityName);
            if(index != -1) {
                mUpdateItems.get(index).check(checked);
            }
        }
    }

    public String getVersion(String capabilityName) {
        int index = containsItem(capabilityName);
        if(mUpdateItems.size() != 0 && index != -1) {
            return mUpdateItems.get(index).getVersion();
        }
        return null;
    }

    public String getServerAddr() {
        return mServerAddress;
    }

    public List<UpdateItem> getCheckedItem() {
        ArrayList<UpdateItem> infos = new ArrayList<UpdateItem>();
        for(UpdateItem item : mUpdateItems) {
            if(item.isChecked()) {
                infos.add(item);
            }
        }
        return infos;
    }

    public List<UpdateItem> getUpdateItems() {
        return mUpdateItems;
    }

    public void setResponseAddr(String responseServer) {
        this.mResponseAddr = responseServer;
    }

    public String getResponseAddr() {
        return mResponseAddr;
    }

    public String getLatsUpdateTime() {
        return mLastUpdateTime;
    }

    public boolean getAutoUpdate() {
        return mIsAutoUpdate;
    }

    public void setonAutoUpdateChanged(autoUpdateChangedListener listener) {
        mAutoupdateListenr = listener;
    }
}
