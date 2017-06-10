/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.GridView;

public class RcsGroupChatDetailMemberListView extends GridView {
    public RcsGroupChatDetailMemberListView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public RcsGroupChatDetailMemberListView(Context context) {
        super(context);
    }

    public RcsGroupChatDetailMemberListView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int expandSpec = MeasureSpec.makeMeasureSpec(Integer.MAX_VALUE >> 2, MeasureSpec.AT_MOST);
        super.onMeasure(widthMeasureSpec, expandSpec);
    }
}
