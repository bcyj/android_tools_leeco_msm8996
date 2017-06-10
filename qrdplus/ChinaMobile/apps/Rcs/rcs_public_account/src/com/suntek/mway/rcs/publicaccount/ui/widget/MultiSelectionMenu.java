/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */
package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.publicaccount.R;

import android.content.Context;
import android.view.View;
import android.widget.Button;

public class MultiSelectionMenu implements View.OnClickListener {
    private final Context mContext;
    private final Button mButton;
    private final MultiPopupList mPopupList;
    public static final int SELECT_OR_DESELECT = 1;

    public MultiSelectionMenu(Context context, Button button,
            MultiPopupList.OnPopupItemClickListener listener) {
        mContext = context;
        mButton = button;
        mPopupList = new MultiPopupList(context, mButton);
        mPopupList.addItem(SELECT_OR_DESELECT, context.getString(R.string.menu_selected_all));
        mPopupList.setOnPopupItemClickListener(listener);
        mButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        mPopupList.show();
    }

    public void dismiss() {
        mPopupList.dismiss();
    }

    public void updateSelectAllMode(boolean inSelectAllMode) {
        MultiPopupList.Item item = mPopupList.findItem(SELECT_OR_DESELECT);
        if (item != null) {
            item.setTitle(mContext.getString(
                    inSelectAllMode ? R.string.menu_cancel_all_selected : R.string.menu_selected_all));
        }
    }

    public void setTitle(CharSequence title) {
        mButton.setText(title);
    }
}
