/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.protectops;

import static com.qapp.secprotect.Configs.MODE_DEPROTECT;
import static com.qapp.secprotect.Configs.MODE_PROTECT;
import static com.qapp.secprotect.Configs.STORAGE_ROOT;
import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.qapp.secprotect.R;
import com.qapp.secprotect.explorer.FileExplorerFragment;

public class ProtectOpsFragment extends Fragment implements OnItemClickListener {

    public static ListView mListView;
    private static View mSettingsView;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        logd("");
        mSettingsView = inflater.inflate(R.layout.authaccess_settings_fragment,
                container, false);
        mListView = (ListView) mSettingsView.findViewById(R.id.authaccess_ops);

        // Prepare the ListView
        CharSequence[] ops = getActivity().getResources().getTextArray(
                R.array.protect_ops);
        String[] protectOps = new String[ops.length];
        for (int i = 0; i < protectOps.length; i++) {
            protectOps[i] = ops[i].toString();
        }

        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                getActivity(), android.R.layout.simple_list_item_1, protectOps);

        mListView.setAdapter(adapter);
        mListView.setOnItemClickListener(this);

        return mSettingsView;
    }

    class OpAdapter extends BaseAdapter {
        ArrayList<String> data;
        Context mContext;

        public OpAdapter(Context context, ArrayList<String> data) {
            this.data = data;
            this.mContext = context;
        }

        @Override
        public int getCount() {
            return data.size();
        }

        @Override
        public Object getItem(int index) {
            return data.get(index);
        }

        @Override
        public long getItemId(int arg0) {
            return arg0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            TextView textView;
            if (convertView == null) {
                textView = new TextView(mContext);
                convertView = textView;
                convertView.setTag(textView);
            } else {
                textView = (TextView) convertView.getTag();
            }

            textView.setText(data.get(position));
            return convertView;
        }

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        logd();
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy() {
        logd();
        super.onDestroy();
    }

    @Override
    public void onResume() {
        logd();
        super.onResume();
    }

    @Override
    public void onPause() {
        logd();
        super.onPause();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        logd();
        super.onSaveInstanceState(outState);
    }

    public static FileExplorerFragment mFileExplorerFragment = null;

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        logd(position);

        switch (position) {
        case 0:
            FileExplorerFragment.init(MODE_PROTECT, STORAGE_ROOT);
            break;
        case 1:
            FileExplorerFragment.init(MODE_DEPROTECT, STORAGE_ROOT);
            break;
        default:
            return;
        }
        getActivity().findViewById(R.id.configs_tabs).setVisibility(View.GONE);
        if (mFileExplorerFragment == null) {
            mFileExplorerFragment = new FileExplorerFragment();
        }

        FragmentTransaction transaction = getFragmentManager()
                .beginTransaction();
        logd("show mFileExplorerFragment");
        transaction.add(android.R.id.tabhost, mFileExplorerFragment,
                "mFileExplorerFragment");
        transaction.commit();

    }
}