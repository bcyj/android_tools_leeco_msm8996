/**
 * Copyright (c) 2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class MenuAdapter extends BaseAdapter {

    private int[] iconsSources;

    private String[] msgsSources;

    private Context mContext;

    public MenuAdapter(Context context, String[] msgs, int... icons) {
        msgsSources = msgs;
        iconsSources = icons;
        mContext = context;
    }

    public int getCount() {
        return msgsSources.length;
    }

    public Object getItem(int position) {
        return msgsSources[position];
    }

    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view = convertView;
        if (view == null)
            view = LayoutInflater.from(mContext).inflate(R.layout.menu_item, null);
        ImageView icon = (ImageView) view.findViewById(R.id.icon);
        icon.setImageResource(iconsSources[position]);
        TextView msg = (TextView) view.findViewById(R.id.msg);
        msg.setText(msgsSources[position]);
        return view;
    }

}
