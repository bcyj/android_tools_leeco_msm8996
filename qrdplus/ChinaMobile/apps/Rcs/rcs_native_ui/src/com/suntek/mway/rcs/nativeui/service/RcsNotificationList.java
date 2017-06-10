/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.service;


import com.suntek.mway.rcs.nativeui.ui.RcsGroupChatInviteNotification;
import com.suntek.mway.rcs.nativeui.ui.RcsNotification;
import com.suntek.mway.rcs.nativeui.utils.RcsNotifyUtil;

import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Collection;

public class RcsNotificationList extends ArrayList<RcsNotification> {
    private static final long serialVersionUID = 1L;
    private static RcsNotificationList instance = new RcsNotificationList();

    public static RcsNotificationList getInstance() {
        return instance;
    }

    private ArrayList<OnChangedListener> listeners = new ArrayList<OnChangedListener>();

    public void addListener(OnChangedListener listener) {
        if (listener != null && !listeners.contains(listener)) {
            listeners.add(listener);
        }
    }

    public void removeListener(OnChangedListener listener) {
        if (listener != null && listeners.contains(listener)) {
            listeners.remove(listener);
        }
    }

    private void notifyChanged() {
        for (OnChangedListener listener : listeners) {
            listener.onChanged();
        }
    }

    public interface OnChangedListener {
        public void onChanged();
    }

    @Override
    public boolean add(RcsNotification object) {
        boolean result = super.add(object);
        RcsNotifyUtil.showNotification();
        notifyChanged();
        return result;
    }

    @Override
    public void add(int index, RcsNotification object) {
        super.add(index, object);
        notifyChanged();
    }

    @Override
    public boolean remove(Object object) {
        boolean result = super.remove(object);
        this.notifyChanged();
        return result;
    }

    @Override
    public RcsNotification remove(int index) {
        RcsNotification result = super.remove(index);
        notifyChanged();
        return result;
    }

    @Override
    public boolean removeAll(Collection<?> collection) {
        boolean result = super.remove(collection);
        notifyChanged();
        return result;
    }

    @Override
    public boolean addAll(Collection<? extends RcsNotification> collection) {
        boolean result = super.addAll(collection);
        notifyChanged();
        return result;
    }

    @Override
    public boolean addAll(int index, Collection<? extends RcsNotification> collection) {
        boolean result = super.addAll(index, collection);
        notifyChanged();
        return result;
    }

    @Override
    public void clear() {
        super.clear();
        notifyChanged();
    }

    @Override
    protected void removeRange(int fromIndex, int toIndex) {
        super.removeRange(fromIndex, toIndex);
        notifyChanged();
    }

    /**
     * Remove the RCS Invite-To-Join-Group Notification if existed.
     */
    public void removeInviteNotificationByChatUri(String chatUri) {
        if (TextUtils.isEmpty(chatUri)) {
            return;
        }

        RcsGroupChatInviteNotification toRemove = null;
        for (RcsNotification notification : this) {
            if (notification instanceof RcsGroupChatInviteNotification) {
                if (chatUri.equals(((RcsGroupChatInviteNotification) notification).getChatUri())) {
                    toRemove = (RcsGroupChatInviteNotification) notification;
                    break;
                }
            }
        }

        if (toRemove != null) {
            remove(toRemove);
        }
    }

    public void removeInviteNotificationBySubject(String subject) {
        if (TextUtils.isEmpty(subject)) {
            return;
        }

        RcsGroupChatInviteNotification toRemove = null;
        for (RcsNotification notification : this) {
            if (notification instanceof RcsGroupChatInviteNotification) {
                if (subject.equals(((RcsGroupChatInviteNotification) notification).getSubject())) {
                    toRemove = (RcsGroupChatInviteNotification) notification;
                    break;
                }
            }
        }

        if (toRemove != null) {
            remove(toRemove);
        }
    }
}
