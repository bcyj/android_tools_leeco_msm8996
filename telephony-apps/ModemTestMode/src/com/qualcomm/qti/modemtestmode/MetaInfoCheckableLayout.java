/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Checkable;
import android.widget.RelativeLayout;
import java.util.ArrayList;
import java.util.List;


public class MetaInfoCheckableLayout extends RelativeLayout implements Checkable {
    private boolean mChecked;
    private List<Checkable> mCheckableList;

    public MetaInfoCheckableLayout(Context context, AttributeSet attrs,
            int defStyle) {
        super(context, attrs, defStyle);
        initCheckable(attrs);
    }

    public MetaInfoCheckableLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        initCheckable(attrs);
    }

    public MetaInfoCheckableLayout(Context context, int checkableId) {
        super(context);
        initCheckable(null);
    }

    public boolean isChecked() {
        return mChecked;
    }

    public void setChecked(boolean checked) {
        this.mChecked = checked;
        for (Checkable c : mCheckableList) {
            c.setChecked(mChecked);
        }
    }

    public void toggle() {
        this.mChecked = !this.mChecked;
        for (Checkable c : mCheckableList) {
            c.toggle();
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        int count = this.getChildCount();
        for (int i = 0; i < count; ++i) {
            getCheckableItem(this.getChildAt(i));
        }
    }

    private void initCheckable(AttributeSet attrs) {
        this.mChecked = false;
        this.mCheckableList = new ArrayList<Checkable>();
    }

    private void getCheckableItem(View v) {
        if (v instanceof Checkable) {
            this.mCheckableList.add((Checkable) v);
        }
        if (v instanceof ViewGroup) {
            ViewGroup vg = (ViewGroup) v;
            int count = vg.getChildCount();
            for (int i = 0; i < count; ++i) {
                getCheckableItem(vg.getChildAt(i));
            }
        }
    }
}
