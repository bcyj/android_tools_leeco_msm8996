/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class MetaInfoListAdapter extends ArrayAdapter <MbnMetaInfo> {
    private ArrayList<MbnMetaInfo> mMbnInfo;
    private Context mContext;

    public MetaInfoListAdapter(Context context, int resource, List<MbnMetaInfo> objects) {
        super(context, resource, objects);
        mContext = context;
        this.mMbnInfo = (ArrayList<MbnMetaInfo>) objects;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View v = convertView;
        if (v == null) {
            LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(
                    Context.LAYOUT_INFLATER_SERVICE);
            v = inflater.inflate(R.layout.meta_info_list, null);
        }

        if (v == null) {
            return null;
        }

        MbnMetaInfo tmp = mMbnInfo.get(position);
        if (tmp != null) {
            TextView metaTv = (TextView) v.findViewById(R.id.meta_info);
            metaTv.setText(tmp.getMetaInfo());

            String mbnVer = mContext.getString(R.string.oem_mbn);
            if (tmp.isQcMbn()) {
                mbnVer = mContext.getString(R.string.qc_mbn);
            } else {
                metaTv.setTextColor(0xf9f93d00);
            }
            TextView sourceTv = (TextView) v.findViewById(R.id.mbn_source);
            sourceTv.setText(mContext.getString(R.string.source) + tmp.getSourceString() +
                    "/" + mbnVer);
        }
        return v;
    }

}
