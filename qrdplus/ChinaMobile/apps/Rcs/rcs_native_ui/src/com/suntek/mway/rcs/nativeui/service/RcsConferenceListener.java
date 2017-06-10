/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.service;

import com.suntek.mway.rcs.nativeui.model.ConferenceEvent;

import android.text.TextUtils;

import java.util.ArrayList;
import java.util.HashMap;

public class RcsConferenceListener {
    private HashMap<String, ArrayList<OnConferenceChangeListener>> listenersMap;
    private static RcsConferenceListener instance = null;

    private RcsConferenceListener() {
        listenersMap = new HashMap<String, ArrayList<OnConferenceChangeListener>>();
    }

    public static RcsConferenceListener getInstance() {
        if (instance == null) {
            instance = new RcsConferenceListener();
        }
        return instance;
    }

    public void addListener(String groupId, OnConferenceChangeListener listener) {
        if (TextUtils.isEmpty(groupId)) {
            return;
        }
        ArrayList<OnConferenceChangeListener> listenerList = listenersMap.get(groupId);
        if (listenerList == null) {
            listenerList = new ArrayList<OnConferenceChangeListener>();
            listenerList.add(listener);
        } else if (!listenerList.contains(listener)) {
            listenerList.add(listener);
        }
        listenersMap.put(groupId, listenerList);
    }

    public void removeListener(String groupId, OnConferenceChangeListener listener) {
        if (TextUtils.isEmpty(groupId)) {
            return;
        }
        ArrayList<OnConferenceChangeListener> listenerList = listenersMap.get(groupId);
        if (listenerList != null && listener != null && listenerList.contains(listener)) {
            listenerList.remove(listener);
        }
    }

    public void notifyChanged(String groupId, ConferenceEvent event) {
        if (TextUtils.isEmpty(groupId) || event == null) {
            return;
        }
        ArrayList<OnConferenceChangeListener> listenerList = listenersMap.get(groupId);
        if (listenerList != null) {
            for (OnConferenceChangeListener listener : listenerList) {
                listener.onChanged(groupId, event);
            }
        }
    }

    public interface OnConferenceChangeListener {
        public void onChanged(String groupId, ConferenceEvent event);
    }

}
