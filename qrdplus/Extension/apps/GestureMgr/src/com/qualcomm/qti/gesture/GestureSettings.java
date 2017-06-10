/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.gesture.Gesture;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.view.MenuItem;
import android.view.View;

public class GestureSettings extends PreferenceActivity {

    public static final String KEY_OFF_SCREEN_GRESTURES = "key_category_off_screen_gestures";
    public static final String KEY_SILENT_GRESTURE = "key_silent_gesture";

    private PreferenceCategory mOffScreenGestures;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.gestures_preference);
        getActionBar().setIcon(Utils.loadSettingsIcon(this));
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setHomeButtonEnabled(true);
        mOffScreenGestures = (PreferenceCategory) findPreference(KEY_OFF_SCREEN_GRESTURES);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        super.onResume();
        loadOffScreenGestures();
    }

    private void loadOffScreenGestures() {
        getPreferenceScreen().removePreference(mOffScreenGestures);
        mOffScreenGestures.removeAll();
        List<GestureAction> gestures = GestureAction.listAvailableActions(this);
        for (GestureAction gestureAction : gestures) {
            if (gestureAction.isValid(this)) {
                GesturePreference preference = new GesturePreference(this);
                preference.setGesture(gestureAction);
                mOffScreenGestures.addPreference(preference);
            }
        }
        if (mOffScreenGestures.getPreferenceCount() != 0) {
            getPreferenceScreen().addPreference(mOffScreenGestures);
        }
    }

    private static class GesturePreference extends Preference {

        private GestureStrokeView mGestureStroke;
        private GestureAction mGestureAction;

        public GesturePreference(Context context) {
            super(context);
            setLayoutResource(R.layout.item_gesture);
        }

        @Override
        protected void onBindView(View view) {
            super.onBindView(view);
            mGestureStroke = (GestureStrokeView) view.findViewById(R.id.gesture_view);
            mGestureStroke.setStrokeColor(getContext().getResources().getColor(
                    R.color.gesture_color));
            bindData();
        }

        public void setGesture(GestureAction gestureAction) {
            mGestureAction = gestureAction;
            bindData();
            setIntent();
        }

        private void setIntent() {
            Intent intent = new Intent(getContext(), GestureSetting.class);
            intent.putExtra(GestureAction.EXTRA_ACTION, mGestureAction.getKey());
            setIntent(intent);
        }

        private void bindData() {
            setTitle(mGestureAction == null ? null : mGestureAction.getActionName(getContext()));
            setSummary(mGestureAction == null ? null
                    : (mGestureAction.isEnabled() ? R.string.switch_on : R.string.switch_off));
            if (mGestureStroke != null) {
                List<Gesture> gestures = null;
                if (mGestureAction != null
                        && (gestures = GestureManagerService.getDefault().getGesture(
                                mGestureAction.getKey())) != null && !gestures.isEmpty()) {
                    mGestureStroke.setGesture(gestures.get(0));
                } else {
                    mGestureStroke.setGesture(null);
                }
            }
        }
    }
}
