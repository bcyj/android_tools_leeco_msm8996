/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.qualcomm.qti.engineertool.Operation.OnOpCompleteListener;
import com.qualcomm.qti.engineertool.model.CheckListItemModel;

public class CheckListAdapter extends BaseAdapter {
    private static final String TAG = "CheckListAdapter";

    private Activity mActivity;
    private CheckListItemModel[] mData;
    private LayoutInflater mInflater;

    public CheckListAdapter(Activity activity, CheckListItemModel[] checkListItemModels) {
        mActivity = activity;
        mData = checkListItemModels;

        mInflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void onItemClick(Fragment target, AdapterView<?> parent, View view, int position,
            long id) {
        Parcelable clickAction = mData[position].getClickAction();
        Utils.onClick(mActivity, target, mData[position], clickAction);
    }

    @Override
    public int getCount() {
        return mData.length;
    }

    @Override
    public Object getItem(int position) {
        return mData[position];
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View v;
        if (convertView == null) {
            v = mInflater.inflate(R.layout.check_list_item, parent, false);
        } else {
            v = convertView;
        }
        bindView(v, position);
        return v;
    }

    private void bindView(View view, int position) {
        final CheckListItemModel itemData = mData[position];

        // Bind the two line view.
        Utils.bindTwoLineView(mActivity, view, itemData);

        TextView valueView = (TextView) view.findViewById(R.id.value);
        ImageView iconView = (ImageView) view.findViewById(R.id.icon);

        String value = itemData.getCurResult();
        if (itemData.needUpdate() || TextUtils.isEmpty(value)) {
            if (TextUtils.isEmpty(value)) {
                valueView.setVisibility(View.GONE);
                iconView.setImageResource(R.drawable.ic_unknown);
            }

            OnOpCompleteListener listener = new OnOpCompleteListener() {
                @Override
                public void onComplete(String result) {
                    if (!TextUtils.isEmpty(result)) {
                        notifyDataSetChanged();
                        itemData.setCurrentResult(result);
                        // Reset the update status as false.
                        itemData.setUpdateStatus(false);
                    }
                }
            };

            Operation op = Operation.getInstance(mActivity.getContentResolver());
            op.doOperationAsync(itemData.getFunction(), itemData.getParams(), listener);
        } else {
            String expect = itemData.getExpectResult();
            if (TextUtils.isEmpty(expect)) {
                iconView.setImageResource(R.drawable.ic_unknown);
            } else {
                int expectType = itemData.getExpextType();
                boolean match = false;
                if ((expectType & CheckListItemModel.EXPECT_TYPE_EQUAL) != 0) {
                    match = match ^ value.matches(expect);
                }
                try {
                    int expectInt = Integer.parseInt(expect);
                    int resultInt = Integer.parseInt(value);
                    if ((expectType & CheckListItemModel.EXPECT_TYPE_LESS) != 0) {
                        match = match ^ (resultInt < expectInt);
                    }
                    if ((expectType & CheckListItemModel.EXPECT_TYPE_MORE) != 0) {
                        match = match ^ (resultInt > expectInt);
                    }
                } catch (NumberFormatException ex) {
                    Log.w(TAG, "Do not support the less or more function.");
                }

                iconView.setImageResource(match ? R.drawable.ic_correct : R.drawable.ic_incorrect);
            }
            valueView.setVisibility(View.VISIBLE);
            String valueLabel = itemData.getSummaryLabel(value);
            valueView.setText(TextUtils.isEmpty(valueLabel) ? value : valueLabel);
        }
    }

}
