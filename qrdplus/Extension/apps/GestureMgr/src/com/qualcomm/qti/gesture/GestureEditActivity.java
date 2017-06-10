/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import android.app.Activity;
import android.content.Intent;
import android.gesture.Gesture;
import android.gesture.GestureOverlayView;
import android.gesture.GestureOverlayView.OnGestureListener;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.widget.TextView;
import android.widget.Toast;

public class GestureEditActivity extends Activity {
    private static final float LENGTH_THRESHOLD = 120.0f;

    private GestureOverlayView mGestureOverlayView;

    private static final int MENU_CLEAR_GESTURE = 1;
    private static final int MENU_SAVE_GESTURE = 2;

    private MenuItem mClearMenuItem;
    private MenuItem mSaveMenuItem;
    private TextView mSummaryView;

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mClearMenuItem = menu.add(Menu.NONE, MENU_CLEAR_GESTURE, 0, R.string.menu_clear);
        mClearMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        mSaveMenuItem = menu.add(Menu.NONE, MENU_SAVE_GESTURE, 0, R.string.menu_ok);
        mSaveMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        boolean valid = mGestureOverlayView.getGesture() != null
                && mGestureOverlayView.getGesture().getStrokesCount() > 0;
        mSaveMenuItem.setEnabled(valid);
        mClearMenuItem.setEnabled(valid);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_SAVE_GESTURE:
                addGesture();
                break;
            case MENU_CLEAR_GESTURE:
                deleteGesture();
                break;
            case android.R.id.home:
                finish();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.create_gesture);
        getActionBar().setIcon(Utils.loadSettingsIcon(this));
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setHomeButtonEnabled(true);
        mSummaryView = (TextView) findViewById(R.id.summary);
        mSummaryView.setText(getString(R.string.summary_gesture_customize, getIntent()
                .getStringExtra(Intent.EXTRA_TEXT)));
        mGestureOverlayView = (GestureOverlayView) findViewById(R.id.gestures_overlay);
        mGestureOverlayView.addOnGestureListener(mOnGestureListener);
        if (getResources().getBoolean(R.bool.flag_multiple_stroke)) {
            mGestureOverlayView.setFadeOffset(GestureManagerService.FADE_OFFSET_GESTURE);
        } else {
            mGestureOverlayView.setFadeOffset(0);
        }
    }

    private void addGesture() {
        if (mGestureOverlayView.getGesture() != null
                && mGestureOverlayView.getGesture().getStrokesCount() > 0) {
            Intent data = new Intent();
            data.putExtra(GestureAction.EXTRA_GESTURE, mGestureOverlayView.getGesture());
            setResult(RESULT_OK, data);
            finish();
        }
    }

    private void deleteGesture() {
        mGestureOverlayView.setGesture(new Gesture());
        invalidateOptionsMenu();
    }

    private OnGestureListener mOnGestureListener = new OnGestureListener() {

        public void onGestureStarted(GestureOverlayView overlay,
                MotionEvent event) {
        }

        public void onGesture(GestureOverlayView overlay, MotionEvent event) {
        }

        public void onGestureEnded(GestureOverlayView overlay, MotionEvent event) {
            if (mGestureOverlayView.getGesture().getLength() < LENGTH_THRESHOLD) {
                overlay.setGesture(new Gesture());
                return;
            }
            String action = GestureManagerService.getDefault().detect(
                    mGestureOverlayView.getGesture());
            if (!TextUtils.isEmpty(action)) {
                Toast.makeText(GestureEditActivity.this, R.string.toast_gesture_exist,
                        Toast.LENGTH_LONG).show();
                overlay.setGesture(new Gesture());
            }
            invalidateOptionsMenu();
        }

        public void onGestureCancelled(GestureOverlayView overlay,
                MotionEvent event) {
        }
    };
}
