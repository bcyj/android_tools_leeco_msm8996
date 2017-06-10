/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.menubar;

import java.util.List;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager.LayoutParams;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfo;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

public class PopUpListView {
    private PopupWindow mPopUpWin = null;
    private View mParent = null;
    TextView yecx = null;
    TextView llcx = null;
    TextView tccx = null;
    LinearLayout main = null;
    LinearLayout menus = null;
    PopUpCallback mCallback;

    @SuppressWarnings("deprecation")
    public PopUpListView(final Context context, final View parent, List<MenuInfo> items,
            PopUpCallback callback) {
        mParent = parent;
        mCallback = callback;
        LayoutInflater inflater = LayoutInflater.from(context);
        View popMainView = (LinearLayout) inflater.inflate(R.layout.popup_list_view, null);
        menus = (LinearLayout) popMainView.findViewById(R.id.menu_items);
        main = (LinearLayout) popMainView.findViewById(R.id.popUpWin);
        mPopUpWin = new PopupWindow(popMainView, LayoutParams.MATCH_PARENT, 500, false);
        mPopUpWin.setBackgroundDrawable(new BitmapDrawable());
        mPopUpWin.setOutsideTouchable(true);

        for (int i = 0, size = items.size(); i < size; i++) {
            MenuInfo item = items.get(i);

            TextView textView = (TextView) inflater.inflate(R.layout.popup_list_item, null);
            textView.setText(item.getTitle());
            if (i > 0) {
                Drawable top = context.getResources().getDrawable(
                        R.drawable.public_account_menu_line);
                top.setBounds(0, 0, top.getMinimumWidth(), top.getMinimumHeight());
                textView.setCompoundDrawables(null, top, null, null);
            }
            textView.setHeight(CommonUtil.dip2px(context, 51));
            textView.setTag(item);
            textView.setOnClickListener(onPopUpItemClickListener);
            menus.addView(textView);
        }

    }

    private OnClickListener onPopUpItemClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            Object tag = v.getTag();
            if (tag instanceof MenuInfo) {
                MenuInfo item = (MenuInfo) tag;
                if (mCallback != null) {
                    //TODO action
                    mCallback.onItemClicked(item);
                }
            }
            dismiss();
        }
    };

    public static interface PopUpCallback {
        public void onItemClicked(MenuInfo menuInfo);
    }

    public void show(int offsetx, int offsety) {
        int[] location = new int[2];
        mParent.getLocationOnScreen(location);
        // mPopUpWin.showAsDropDown(mParent);
        mPopUpWin.showAtLocation(mParent, Gravity.TOP, 0, location[1] - mPopUpWin.getHeight());
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT
                , LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.leftMargin = location[0];
        menus.setLayoutParams(lp);
    }

    public void dismiss() {
        if (mPopUpWin.isShowing()) {
            mPopUpWin.dismiss();
        }
    }

}
