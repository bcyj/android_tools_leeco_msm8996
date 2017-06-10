/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.service.RcsNotificationList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class RcsNotificationListAdapter extends BaseAdapter {
    private RcsNotificationList mNotifications;
    private Context mContext;
    private final LayoutInflater mFactory;

    public RcsNotificationListAdapter(Context context, RcsNotificationList notifications) {
        mContext = context;
        mNotifications = notifications;
        mFactory = LayoutInflater.from(context);
    }

    @Override
    public int getCount() {
        return mNotifications.size();
    }

    @Override
    public RcsNotification getItem(int position) {
        return mNotifications.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = mFactory.inflate(R.layout.rcs_notification_list_item, parent, false);
        }

        RcsNotificationListItem headerView = (RcsNotificationListItem) convertView;

        RcsNotification notification = getItem(position);

        headerView.bind(mContext, notification);

        return convertView;
    }

}
