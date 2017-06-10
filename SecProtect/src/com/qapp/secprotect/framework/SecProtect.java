/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.framework;

import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.util.DisplayMetrics;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.GridLayoutAnimationController;
import android.view.animation.ScaleAnimation;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qapp.secprotect.R;
import com.qapp.secprotect.utils.UtilsSystem;

public class SecProtect extends PreferenceActivity implements
        OnItemClickListener {

    private Context mContext;
    private static final String SECPROTECT_INTENT = "qapp.intent.action.secprotect";
    private List<ResolveInfo> mGridList;
    GestureDetector mGestureDetector;
    DisplayMetrics displayMetrics = new DisplayMetrics();
    Map<String, ?> mMap = new HashMap<String, Integer>();
    GridAdapter mGridAdapter;
    /** View */
    private GridView mGridView;
    private TextView mStatusTextView;

    private List<ResolveInfo> loadGridList() {
        List<ResolveInfo> gridList;
        Intent mainIntent = new Intent(SECPROTECT_INTENT, null);
        gridList = getPackageManager().queryIntentActivities(mainIntent, 0);

        return gridList;
    }

    private AnimationSet makeAnimation() {
        AnimationSet animationSet = new AnimationSet(false);

        Animation scaleAnimation1 = new ScaleAnimation(0.5f, 1.5f, 0.5f, 1.5f,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF,
                0.5f);
        scaleAnimation1.setDuration(500);
        animationSet.addAnimation(scaleAnimation1);

        Animation scaleAnimation2 = new ScaleAnimation(1.5f, 1.0f, 1.5f, 1.0f,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF,
                0.5f);
        scaleAnimation2.setDuration(500);
        animationSet.addAnimation(scaleAnimation2);

        AlphaAnimation alphaAnimation = new AlphaAnimation(0.0f, 1.0f);
        alphaAnimation.setDuration(500);
        animationSet.addAnimation(alphaAnimation);

        return animationSet;
    }

    void init() {
        mContext = getApplicationContext();
        getActionBar().setDisplayShowHomeEnabled(false);

        if (this instanceof PreferenceActivity) {
            addPreferencesFromResource(R.xml.secprotect_prefs);
        } else if (this instanceof Activity) {
            // use gridview
            setContentView(R.layout.secprotect);

            getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

            mStatusTextView = (TextView) findViewById(R.id.secprotect_status);
            mGridView = (GridView) findViewById(R.id.secprotect_gridview);
            mStatusTextView.setPadding(displayMetrics.widthPixels / 2, 0, 0, 0);

            mGridList = loadGridList();
            logd("EntranceNum=" + mGridList.size());
            mGridAdapter = new GridAdapter(mGridList);
            mGridView.setAdapter(mGridAdapter);
            mGridView.setOnItemClickListener(this);

            GridLayoutAnimationController gridLayoutAnimationController = new GridLayoutAnimationController(
                    makeAnimation());
            gridLayoutAnimationController.setRowDelay(0.75f);
            gridLayoutAnimationController.setColumnDelay(0f);
            gridLayoutAnimationController
                    .setDirectionPriority(GridLayoutAnimationController.PRIORITY_NONE);

            mGridView.setLayoutAnimation(gridLayoutAnimationController);
            mGridView.setColumnWidth(displayMetrics.widthPixels / 2);
            mGridView.setNumColumns(2);
            mGridView.setPadding(0, displayMetrics.widthPixels / 10, 0, 0);
            mGridView.setVerticalSpacing(displayMetrics.heightPixels / 12);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (UtilsSystem.isMonkeyRunning()) {
            return;
        }
        init();
    }

    @Override
    protected void onResume() {
        logd("");
        refreshUI();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("");
        registerReceiver(mBroadcastReceiver, intentFilter);
        super.onResume();
    }

    @Override
    protected void onPause() {
        logd("");
        unregisterReceiver(mBroadcastReceiver);
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        logd("");
        super.onDestroy();
    };

    @Override
    public void onItemClick(AdapterView<?> parent, View view,
            final int position, long id) {
        ResolveInfo resolveInfo = (ResolveInfo) mGridView
                .getItemAtPosition(position);
        ComponentName componentName = new ComponentName(
                resolveInfo.activityInfo.applicationInfo.packageName,
                resolveInfo.activityInfo.name);
        Intent intent = new Intent();
        intent.setComponent(componentName);
        startActivity(intent);
    }

    private class GridAdapter extends BaseAdapter {

        private List<ResolveInfo> list;

        public GridAdapter(List<ResolveInfo> list) {
            this.list = list;
        }

        public int getCount() {
            if (list == null) {
                return 0;
            }
            return list.size();
        }

        public Object getItem(int position) {
            if (list == null) {
                return null;
            }
            return list.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(final int position, View convertView,
                ViewGroup parent) {

            ItemView itemView = null;
            if (convertView != null)
                itemView = (ItemView) convertView;
            else {
                itemView = (ItemView) LayoutInflater.from(mContext).inflate(
                        R.layout.itemview, null);
            }
            ResolveInfo resolveInfo = list.get(position);
            ImageView iconImageView = (ImageView) itemView
                    .findViewById(R.id.itemview_icon);
            TextView nameTextView = (TextView) itemView
                    .findViewById(R.id.itemview_name);

            nameTextView.setText(resolveInfo.activityInfo
                    .loadLabel(getPackageManager()));
            iconImageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
            iconImageView.setLayoutParams(new LinearLayout.LayoutParams(
                    displayMetrics.widthPixels / 7,
                    displayMetrics.widthPixels / 7));
            nameTextView.setTextColor(Color.WHITE);
            iconImageView.setBackground(resolveInfo.activityInfo
                    .loadIcon(getPackageManager()));

            return itemView;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mGestureDetector != null) {
            if (mGestureDetector.onTouchEvent(event))
                return true;
            else
                return false;
        } else
            return false;
    }

    private void notifyDataChanged() {
        mGridList = loadGridList();
        mGridAdapter.notifyDataSetChanged();
    }

    private void refreshUI() {
        logd("");
        mGridList = loadGridList();
        if (mGridView != null && mGridAdapter != null) {
            mGridView.setOnItemClickListener(this);
            mGridView.setAdapter(mGridAdapter);
            mGridAdapter.notifyDataSetChanged();
        }
    }

    BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd(action);
        };
    };

    // private static final int ADVANCED_MENU = Menu.FIRST;
    //
    // public boolean onCreateOptionsMenu(Menu menu) {
    // logd("");
    // SubMenu addMenu = menu.addSubMenu(0, ADVANCED_MENU, 0,
    // R.string.advanced);
    // addMenu.setIcon(android.R.drawable.ic_menu_add);
    // return true;
    // }
    //
    // @Override
    // public boolean onOptionsItemSelected(MenuItem item) {
    // switch (item.getItemId()) {
    // case ADVANCED_MENU:
    // break;
    // default:
    // break;
    // }
    // return super.onOptionsItemSelected(item);
    // }

}
