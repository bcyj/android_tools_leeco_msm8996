/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import java.util.List;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.Intent;
import android.gesture.Gesture;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

public class GestureSetting extends PreferenceActivity implements OnCheckedChangeListener,
        OnPreferenceClickListener {

    private static final String TAG = "GestureSetting";

    private GestureAction mGestureAction;
    private String mAction;
    private Switch mSwitch;
    private GesturePreference mGesturePreference;
    private Preference mCustomizePreference;
    private Preference mLabelPreference;
    private Gesture mGesture;
    private boolean edited;

    private static final String KEY_GESTURE_NAME = "key_gesture_name";
    private static final String KEY_GESTURE_CUSTOMIZE = "key_gesture_customize";

    private static final int MENU_ID_SAVE = 1;
    private static final int MENU_ID_DISCARD = 3;
    private static final int MENU_ID_RESET_DEFAULT = 2;

    private static final int RESULT_CODE_GESTURE = 1;

    private MenuItem mResetMenuItem;
    private MenuItem mSaveMenuItem;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.gesture_setting);
        getActionBar().setIcon(Utils.loadSettingsIcon(this));
        mAction = getIntent().getStringExtra(GestureAction.EXTRA_ACTION);
        mGestureAction = GestureAction.queryAction(mAction);
        if (mGestureAction == null) {
            finish();
            return;
        }
        loadGesture();
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setHomeButtonEnabled(true);
        mSwitch = new Switch(this);
        mSwitch.setOnCheckedChangeListener(this);
        if (onIsHidingHeaders() || !onIsMultiPane()) {
            final int padding = getResources().getDimensionPixelSize(
                    R.dimen.action_bar_switch_padding);
            mSwitch.setPaddingRelative(0, 0, padding, 0);
            getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                    ActionBar.DISPLAY_SHOW_CUSTOM);
            getActionBar().setCustomView(mSwitch, new ActionBar.LayoutParams(
                    ActionBar.LayoutParams.WRAP_CONTENT,
                    ActionBar.LayoutParams.WRAP_CONTENT,
                    Gravity.CENTER_VERTICAL | Gravity.END));
        }
        mCustomizePreference = findPreference(KEY_GESTURE_CUSTOMIZE);
        mLabelPreference = findPreference(KEY_GESTURE_NAME);
        mCustomizePreference.setOnPreferenceClickListener(this);
        mGesturePreference = new GesturePreference();
        getPreferenceScreen().addPreference(mGesturePreference);
    }

    private void loadGesture() {
        List<Gesture> gestures = GestureManagerService.getDefault().getGesture(
                mGestureAction.getKey());
        if (gestures != null && !gestures.isEmpty()) {
            mGesture = gestures.get(0);
        }
    }

    private void customizeGesture() {
        Intent intent = new Intent(this, GestureEditActivity.class);
        intent.putExtra(Intent.EXTRA_TEXT, mGestureAction.getActionName(this));
        startActivityForResult(intent, RESULT_CODE_GESTURE);
    }

    private void updateUI() {
        boolean enable = mGestureAction != null && mGestureAction.isEnabled();
        mSwitch.setChecked(mGestureAction.isEnabled());
        mLabelPreference.setSummary(mGestureAction.getActionName(this));
        mGesturePreference.bindData();
        getPreferenceScreen().setEnabled(enable);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateUI();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mSaveMenuItem = menu.add(Menu.NONE, MENU_ID_SAVE, 0, R.string.menu_save);
        mSaveMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        mResetMenuItem = menu.add(Menu.NONE, MENU_ID_RESET_DEFAULT, 0, R.string.menu_reset);
        mResetMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
        menu.add(Menu.NONE, MENU_ID_DISCARD, 0, R.string.menu_discard)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        mSaveMenuItem.setEnabled(edited);
        mResetMenuItem.setEnabled(!Utils.isGestureSame(GestureManagerService.getDefault()
                .getDefault(mAction), mGesture));
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public void onBackPressed() {
        if (save()) {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_ID_SAVE:
            case android.R.id.home:
                if (save()) {
                    finish();
                }
                break;
            case MENU_ID_DISCARD:
                if (edited) {
                    new AlertDialogFragment(R.string.message_discard).show(getFragmentManager(),
                            TAG);
                } else {
                    finish();
                }
                break;
            case MENU_ID_RESET_DEFAULT:
                new AlertDialogFragment(R.string.message_reset_default).show(getFragmentManager(),
                        TAG);
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    public class AlertDialogFragment extends DialogFragment {

        private int mMessageResId;

        public AlertDialogFragment(int messageResId) {
            mMessageResId = messageResId;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            if (mMessageResId != 0) {
                builder.setMessage(mMessageResId);
            }
            builder.setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            switch (mMessageResId) {
                                case R.string.message_discard:
                                    finish();
                                    break;
                                case R.string.message_reset_default:
                                    setGesture(GestureManagerService.getDefault().getDefault(
                                            mAction));
                                    break;
                            }
                        }
                    });
            builder.setNegativeButton(android.R.string.cancel, null);
            return builder.create();
        }
    }

    private void setGesture(Gesture gesture) {
        if (gesture != null) {
            mGesture = gesture;
            mGesturePreference.bindData();
            edited = true;
        }
        invalidateOptionsMenu();
    }

    private boolean save() {
        if (!edited) {
            return true;
        }
        boolean result = mGesture != null
                && GestureManagerService.getDefault().saveGesture(mAction, mGesture);
        Toast.makeText(this, result ? R.string.toast_gesture_saved : R.string.toast_gesture_error,
                Toast.LENGTH_SHORT).show();
        return result;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case RESULT_CODE_GESTURE:
                    if (data != null) {
                        setGesture((Gesture) data.getParcelableExtra(GestureAction.EXTRA_GESTURE));
                    }
                    break;
            }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    private class GesturePreference extends Preference {

        private GestureStrokeView mGestureStroke;
        private TextView mSummaryView;

        public GesturePreference() {
            super(GestureSetting.this);
            setLayoutResource(R.layout.gesture_view_for_setting);
        }

        @Override
        protected void onBindView(View view) {
            super.onBindView(view);
            mGestureStroke = (GestureStrokeView) view.findViewById(R.id.gesture_view);
            mGestureStroke.setStrokeColor(getContext().getResources().getColor(
                    R.color.gesture_color));
            mGestureStroke.setStrokeWith(10.0f);
            mSummaryView = (TextView) view.findViewById(R.id.summary);
            bindData();
        }

        private void bindData() {
            if (mGestureStroke != null) {
                mGestureStroke.setGesture(mGesture);
            }
            if (mSummaryView != null) {
                if (mGestureAction != null) {
                    mSummaryView.setText(getString(R.string.summary_gesture_setting,
                            mGestureAction.getActionName(GestureSetting.this)));
                } else {
                    mSummaryView.setText(null);
                }
            }
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (mGestureAction != null && isChecked != mGestureAction.isEnabled()) {
            mGestureAction.setEnable(isChecked);
            if (mGestureAction.save()) {
                GestureManagerService.getDefault().updateDetectState();
            } else {
                mGestureAction.setEnable(!isChecked);
            }
            updateUI();
        }
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (mCustomizePreference == preference) {
            customizeGesture();
        }
        return false;
    }
}
