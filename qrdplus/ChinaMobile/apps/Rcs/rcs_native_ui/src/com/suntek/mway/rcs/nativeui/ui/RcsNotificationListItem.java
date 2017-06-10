/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.service.RcsNotificationList;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;
import android.widget.QuickContactBadge;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class RcsNotificationListItem extends RelativeLayout {
    private TextView mSubjectView;
    private TextView mFromView;
    private QuickContactBadge mAvatarView;
    private Button mAcceptView;
    private Button mDeclineView;

    public RcsNotificationListItem(Context context) {
        super(context);
    }

    public RcsNotificationListItem(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public RcsNotificationListItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public final void bind(Context context, RcsNotification notification) {
        mFromView.setText(notification.getSubject());
        mSubjectView.setText(notification.getText(getContext()));
        if (notification.getIsChairmanChange()) {
            mAcceptView.setVisibility(View.VISIBLE);
            mAcceptView.setText(R.string.confirm);
            mDeclineView.setVisibility(View.GONE);
            OnClickListener l = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    RcsNotification notification = (RcsNotification) v.getTag();
                    RcsNotificationList.getInstance().remove(notification);
                }
            };
            mAcceptView.setTag(notification);
            mAcceptView.setOnClickListener(l);
        } else {
            mAcceptView.setVisibility(View.VISIBLE);
            mDeclineView.setVisibility(View.VISIBLE);
            mAcceptView.setTag(notification);
            mDeclineView.setTag(notification);
            OnClickListener l = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    RcsNotification notification = (RcsNotification) v.getTag();
                    switch (v.getId()) {
                    case R.id.accept:
                        notification.onPositiveButtonClicked();
                        break;
                    case R.id.decline:
                        notification.onNegativeButtonClicked();
                        break;
                    }
                }
            };
            mAcceptView.setOnClickListener(l);
            mDeclineView.setOnClickListener(l);
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mFromView = (TextView) findViewById(R.id.from);
        mSubjectView = (TextView) findViewById(R.id.subject);
        mAvatarView = (QuickContactBadge) findViewById(R.id.avatar);
        mAcceptView = (Button) findViewById(R.id.accept);
        mDeclineView = (Button) findViewById(R.id.decline);

        mAcceptView.setFocusable(false);
        mDeclineView.setFocusable(false);
    }
}
