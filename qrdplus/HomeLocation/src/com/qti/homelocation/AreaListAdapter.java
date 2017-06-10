/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import android.content.Context;
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.CursorTreeAdapter;
import android.widget.TextView;

import com.qti.homelocation.GeocodedLocation.Area;
import com.qti.homelocation.GeocodedLocation.AreaCode;

public class AreaListAdapter extends CursorTreeAdapter {

    private Context mContext;
    private float mDesnsity;
    private static final int AREA_PADDING_LEFT = 40;
    private static final int AREA_PADDING_TOP = 20;
    private static final int AREA_PADDING_RIGHT = 0;
    private static final int AREA_PADDING_BOTTOM = 20;
    private static final int TEXT_SIZE = 15;
    private static final float FLOATING_VALUE = 0.5f;

    public AreaListAdapter(Context context) {
        super(context.getContentResolver().query(
                Area.CONTENT_URI, null, null, null, null), context);
        mContext = context;
        mDesnsity = context.getResources().getDisplayMetrics().density;
    }

    @Override
    protected Cursor getChildrenCursor(Cursor groupCursor) {
        Area area = new Area(groupCursor);
        return mContext.getContentResolver().query(AreaCode.CONTENT_URI, null,
                AreaCode.AREA_ID + "=?", new String[] {
                    String.valueOf(area.getAreaId())
                }, AreaCode.CODE);
    }

    @Override
    protected View newGroupView(Context context, Cursor cursor,
            boolean isExpanded, ViewGroup parent) {
        View view = LayoutInflater.from(context).inflate(
                android.R.layout.simple_expandable_list_item_1, null);
        view.setLayoutParams(
                new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT,
                AbsListView.LayoutParams.WRAP_CONTENT));
        return view;
    }

    @Override
    protected void bindGroupView(View view, Context context, Cursor cursor, boolean isExpanded) {
        Area area = new Area(cursor);
        TextView textView = (TextView) view.findViewById(android.R.id.text1);
        textView.setLayoutParams(
                new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT,
                AbsListView.LayoutParams.WRAP_CONTENT));
        textView.setPadding((int) (AREA_PADDING_LEFT * mDesnsity + FLOATING_VALUE),
                AREA_PADDING_TOP, AREA_PADDING_RIGHT, AREA_PADDING_BOTTOM);
        textView.setTextSize(TEXT_SIZE);
        textView.setText(area.getName());
    }

    @Override
    protected View newChildView(Context context, Cursor cursor, boolean isLastChild,
            ViewGroup parent) {
        return LayoutInflater.from(context).inflate(
                android.R.layout.simple_expandable_list_item_2, null);
    }

    @Override
    protected void bindChildView(View view, Context context, Cursor cursor, boolean isLastChild) {
        AreaCode areaCode = new AreaCode(cursor);
        TextView nameView = (TextView) view.findViewById(android.R.id.text1);
        TextView codeView = (TextView) view.findViewById(android.R.id.text2);
        nameView.setTextSize(TEXT_SIZE);
        codeView.setTextSize(TEXT_SIZE);
        codeView.setTextColor(android.graphics.Color.GRAY);
        nameView.setText(areaCode.getCity());
        codeView.setText(areaCode.getCode());
    }

}
