/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Activity;
import android.content.Context;
import android.os.Parcelable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.ExpandableListView;

import com.qualcomm.qti.engineertool.model.GroupModel;

public class GroupListAdapter extends BaseExpandableListAdapter {
    private Activity mActivity;
    private GroupModel[] mData;
    private LayoutInflater mInflater;

    public GroupListAdapter(Activity activity, GroupModel[] data) {
        mActivity = activity;
        mData = data;

        mInflater = (LayoutInflater) mActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
            int childPosition, long id) {
        Parcelable clickAction = mData[groupPosition].getChild(childPosition).getClickAction();
        return Utils.onClick(mActivity, null, null, clickAction);
    }

    @Override
    public int getGroupCount() {
        return mData.length;
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        return mData[groupPosition].getChildrenCount();
    }

    @Override
    public Object getGroup(int groupPosition) {
        return mData[groupPosition];
    }

    @Override
    public Object getChild(int groupPosition, int childPosition) {
        return mData[groupPosition].getChild(childPosition);
    }

    @Override
    public long getGroupId(int groupPosition) {
        return groupPosition;
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return childPosition;
    }

    @Override
    public boolean hasStableIds() {
        return true;
    }

    @Override
    public View getGroupView(int groupPosition, boolean isExpanded, View convertView,
            ViewGroup parent) {
        View v;
        if (convertView == null) {
            v = mInflater.inflate(R.layout.simple_list_item, parent, false);
        } else {
            v = convertView;
        }
        bindGroupView(v, groupPosition, isExpanded);
        return v;
    }

    @Override
    public View getChildView(int groupPosition, int childPosition, boolean isLastChild,
            View convertView, ViewGroup parent) {
        View v;
        if (convertView == null) {
            v = mInflater.inflate(R.layout.simple_list_item, parent, false);
        } else {
            v = convertView;
        }
        bindChildView(v, groupPosition, childPosition, isLastChild);
        return v;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return true;
    }

    private void bindGroupView(View view, int groupPosition, boolean isExpanded) {
        Utils.bindTwoLineView(mActivity, view, mData[groupPosition]);
    }

    private void bindChildView(View view, int groupPosition, int childPosition,
            boolean isLastChild) {
        Utils.bindTwoLineView(mActivity, view, mData[groupPosition].getChild(childPosition));
    }
}
