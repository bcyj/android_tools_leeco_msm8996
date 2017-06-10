/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.appsettings;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

import com.qapp.secprotect.R;
import com.qapp.secprotect.authaccess.UtilsDatabase;
import com.qapp.secprotect.data.AuthInfo;

public class AppListAdapter extends ArrayAdapter<AppRecord> {
    private final LayoutInflater mInflater;
    private final Context mContext;

    public AppListAdapter(Context context) {
        super(context, android.R.layout.simple_list_item_2);
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mContext = context;
    }

    public void setData(List<AppRecord> data) {
        clear();
        if (data != null) {
            for (AppRecord appRecord : data) {
                add(appRecord);
            }
        }
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view;

        if (convertView == null) {
            view = mInflater.inflate(R.layout.list_item_authaccess_settings, parent, false);
        } else {
            view = convertView;
        }

        AppRecord appRecord = getItem(position);
        final AuthInfo authInfo = appRecord.getAuthInfo();
        ((ImageView) view.findViewById(R.id.app_icon)).setImageDrawable(appRecord.getIcon());
        ((TextView) view.findViewById(R.id.app_name)).setText(appRecord.getLabel());

        Spinner spinner = (Spinner) view.findViewById(R.id.grant_spinner);
        switch (authInfo.mode) {
        case -1:
            spinner.setSelection(2, true);
            break;
        case 1:
            spinner.setSelection(1, true);
            break;
        default:
            spinner.setSelection(0, true);
            break;
        }
        spinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                authInfo.mode = 1;
                switch (position) {
                case 0:
                    authInfo.mode = 0;
                    break;
                case 1:
                    authInfo.mode = 1;
                    break;
                case 2:
                    authInfo.mode = -1;
                    break;
                }
                UtilsDatabase.setGrantMode(mContext, authInfo);
            }
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        return view;
    }
}