/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.menubar;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.MenuInfo;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.ui.menubar.PopUpListView.PopUpCallback;

public class MessageMenuBar implements OnClickListener {

    public static final int TYPE_MSG = 0;

    public static final int TYPE_URL = 1;

    public static final int TYPE_DEVICE_API = 2;

    public static final int TYPE_APP = 3;

    public interface onMenuBarClickListener {
        void onActionClick(MenuInfo menuInfo);
    }

    private LinearLayout layoutInput, layoutMenu;

    private View layoutMenu1, layoutMenu2, layoutMenu3;

    private TextView textMenu1, textMenu2, textMenu3;

    private ImageView imageMenu1, imageMenu2, imageMenu3;

    private View btnSwitchToMsg, btnSwitchToMenu;

    private Activity context;

    private List<MenuInfo> menuList = new ArrayList<MenuInfo>();

    private onMenuBarClickListener menuBarListener;

    public MessageMenuBar(Activity context, View layout, onMenuBarClickListener actionListener) {
        this.context = context;
        layoutInput = (LinearLayout)layout.findViewById(R.id.linearLayout2);
        layoutMenu = (LinearLayout)layout.findViewById(R.id.pub_account_menus);
        layoutMenu1 = layout.findViewById(R.id.layout_menu1);
        layoutMenu2 = layout.findViewById(R.id.layout_menu2);
        layoutMenu3 = layout.findViewById(R.id.layout_menu3);
        textMenu1 = (TextView)layout.findViewById(R.id.text_item1);
        textMenu2 = (TextView)layout.findViewById(R.id.text_item2);
        textMenu3 = (TextView)layout.findViewById(R.id.text_item3);
        imageMenu1 = (ImageView)layout.findViewById(R.id.image_item1);
        imageMenu2 = (ImageView)layout.findViewById(R.id.image_item2);
        imageMenu3 = (ImageView)layout.findViewById(R.id.image_item3);
        btnSwitchToMsg = layout.findViewById(R.id.btnSwitchToMsg);
        btnSwitchToMenu = layout.findViewById(R.id.btnSwitchToMenu);

        layout.findViewById(R.id.btn_Send).setOnClickListener(this);
        layoutMenu1.setOnClickListener(this);
        layoutMenu2.setOnClickListener(this);
        layoutMenu3.setOnClickListener(this);

        this.menuBarListener = actionListener;
    }

    public void initMenuBar(List<MenuInfo> menuList) {
        this.menuList.clear();
        this.menuList.addAll(menuList);
        layoutMenu.setVisibility(View.VISIBLE);
        layoutInput.setVisibility(View.GONE);
        btnSwitchToMenu.setVisibility(View.VISIBLE);

        int count = menuList.size();
        if (count > 0) {
            layoutMenu1.setVisibility(View.VISIBLE);
            textMenu1.setText(menuList.get(0).getTitle());
            if (hasSubMenu(this.menuList.get(0))) {
                imageMenu1.setVisibility(View.VISIBLE);
            } else {
                imageMenu1.setVisibility(View.GONE);
            }
        } else {
            layoutMenu1.setVisibility(View.GONE);
            textMenu1.setText("");
        }
        if (count > 1) {
            layoutMenu2.setVisibility(View.VISIBLE);
            textMenu2.setText(menuList.get(1).getTitle());
            if (hasSubMenu(this.menuList.get(1))) {
                imageMenu2.setVisibility(View.VISIBLE);
            } else {
                imageMenu2.setVisibility(View.GONE);
            }
        } else {
            layoutMenu2.setVisibility(View.GONE);
            textMenu2.setText("");
        }
        if (count > 2) {
            layoutMenu3.setVisibility(View.VISIBLE);
            textMenu3.setText(menuList.get(2).getTitle());
            // has sub menu or no
            if (hasSubMenu(this.menuList.get(2))) {
                imageMenu3.setVisibility(View.VISIBLE);
            } else {
                imageMenu3.setVisibility(View.GONE);
            }
        } else {
            layoutMenu3.setVisibility(View.GONE);
            textMenu3.setText("");
        }

        btnSwitchToMsg.setOnClickListener(this);
        btnSwitchToMenu.setOnClickListener(this);
    }

    public void onMenuNull() {
        layoutMenu.setVisibility(View.GONE);
        layoutInput.setVisibility(View.VISIBLE);

        btnSwitchToMenu.setVisibility(View.GONE);
    }

    private void switchToMsg() {
        layoutMenu.setVisibility(View.GONE);
        layoutInput.setVisibility(View.VISIBLE);
    }

    private void switchToMenu() {
        layoutMenu.setVisibility(View.VISIBLE);
        layoutInput.setVisibility(View.GONE);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnSwitchToMsg:
                switchToMsg();
                break;
            case R.id.btnSwitchToMenu:
                switchToMenu();
                break;
            case R.id.layout_menu1:
                if(menuList == null || menuList.size() < 1 )
                    return;
                if (hasSubMenu(menuList.get(0))) {
                    PopUpListView list1 = new PopUpListView(context, layoutMenu1, menuList.get(0)
                            .getSubMenuList(), callBack);
                    list1.show(0, 0);
                } else {
                    handlerMenuAction(menuList.get(0));
                }
                break;
            case R.id.layout_menu2:
                if(menuList == null || menuList.size() < 2 )
                    return;
                if (hasSubMenu(menuList.get(1))) {
                    PopUpListView list2 = new PopUpListView(context, layoutMenu2, menuList.get(1)
                            .getSubMenuList(), callBack);
                    list2.show(0, 0);
                } else {
                    handlerMenuAction(menuList.get(1));
                }
                break;
            case R.id.layout_menu3:
                if(menuList == null || menuList.size() < 3 )
                    return;
                if (hasSubMenu(menuList.get(2))) {
                    PopUpListView list3 = new PopUpListView(context, layoutMenu3, menuList.get(2)
                            .getSubMenuList(), callBack);
                    list3.show(0, 0);
                } else {
                    handlerMenuAction(menuList.get(2));
                }
                break;
        }
    }

    private boolean hasSubMenu(MenuInfo info) {
        if (info == null) {
            return false;
        }
        List<MenuInfo> subMenuList = info.getSubMenuList();
        if (subMenuList != null && subMenuList.size() > 0) {
            return true;
        }
        return false;
    }

    private void handlerMenuAction(MenuInfo menuInfo) {
        if (menuBarListener != null) {
            menuBarListener.onActionClick(menuInfo);
        }
    }

    private PopUpCallback callBack = new PopUpCallback() {

        @Override
        public void onItemClicked(MenuInfo menuInfo) {
            if (menuBarListener != null) {
                menuBarListener.onActionClick(menuInfo);
            }
        }
    };
}
