/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file SviSample.java
 *
 */
package com.qualcomm.snapdragon.sample;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.ADAPTIVE_BACKLIGHT_QUALITY_LEVEL;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_FEATURE;

public class AbFragment extends Fragment {

    private static String TAG = "DisplaySDK-AbFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;
    private RadioGroup abRadioGroup = null;
    private RadioButton abRadioHigh = null;
    private RadioButton abRadioMedium = null;
    private RadioButton abRadioLow = null;
    private RadioButton abRadioAuto = null;
    private TextView abStatus = null;
    private Button abRefresh = null;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.ab_sample, container, false);
        appContext = getActivity().getApplicationContext();
        updateBackgroundImage();
        setHasOptionsMenu(true);

        if (null != cMgr) {
            // Log.i(TAG,
            // "onCreateView: Valid color manager object, initializing the UI");
            getColorManagerInstance();
        } else {
            Log.e(TAG, "onCreateView: null color manager object");
        }
        return rootView;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        // Adaptive backlight is not saved as part of the mode so hide
        // the item for save mode
        menu.findItem(R.id.save_mode).setVisible(false);
        // Log.i(TAG, "Hiding Save mode button");
    }

    public void updateBackgroundImage() {
        if (null != rootView) {
            ImageView im = (ImageView) rootView.findViewById(R.id.surfaceimage);
            if (null != MenuActivity.bitmap) {
                im.setImageBitmap(MenuActivity.bitmap);
            }
        }
    }

    public void colorManagerInit(ColorManager mgr) {
        if (null != mgr) {
            cMgr = mgr;
            if (this.isVisible()) {
                // The color manager service just connected and this fragment is
                // visible initialize the UI
                // Log.i(TAG,
                // "colorManagerInit - visible fragment, initializing UI");
                getColorManagerInstance();
            }
        }
    }

    private void selectRadioButton() {
        ADAPTIVE_BACKLIGHT_QUALITY_LEVEL level = cMgr
                .getBacklightQualityLevel();
        if (null != level) {
            switch (level) {
            case AUTO:
                abRadioAuto.setChecked(true);
                // Log.i(TAG, "selectRadioButton set to Auto");
                break;
            case HIGH:
                abRadioHigh.setChecked(true);
                // Log.i(TAG, "selectRadioButton set to High");
                break;
            case MEDIUM:
                abRadioMedium.setChecked(true);
                // Log.i(TAG, "selectRadioButton set to Medium");
                break;
            case LOW:
                abRadioLow.setChecked(true);
                // Log.i(TAG, "selectRadioButton set to Low");
                break;
            default:
                Log.e(TAG, "selectRadioButton called with illegal parameter");
            }
        }
    }

    private void setBacklightLvl(String level) {
        // Log.i(TAG,
        // "setBacklightLvl - updating backlight status to display level ="
        // + level);
        if (getResources().getString(R.string.ab_auto) == level) {
            cMgr.setBacklightQualityLevel(ADAPTIVE_BACKLIGHT_QUALITY_LEVEL.AUTO);
        } else if (getResources().getString(R.string.ab_high) == level) {
            cMgr.setBacklightQualityLevel(ADAPTIVE_BACKLIGHT_QUALITY_LEVEL.HIGH);
        } else if (getResources().getString(R.string.ab_medium) == level) {
            cMgr.setBacklightQualityLevel(ADAPTIVE_BACKLIGHT_QUALITY_LEVEL.MEDIUM);
        } else if (getResources().getString(R.string.ab_low) == level) {
            cMgr.setBacklightQualityLevel(ADAPTIVE_BACKLIGHT_QUALITY_LEVEL.LOW);
        } else {
            Log.e(TAG, "setBacklightLvl called with illegal parameter");
        }
    }

    private void updateBacklightStatus() {
        // Log.i(TAG,
        // "updateBacklightStatus: updating adaptive backlight status to UI");
        int scale = cMgr.getAdaptiveBacklightScale();
        if (scale<0)
            scale = 0;
        String displayString = getResources().getString(R.string.ab_mode);
        abStatus.setText(displayString + ": "
                + scale + "%");
    }

    private void setEnableUI(boolean checked){
        abRadioGroup.setEnabled(checked);
        abRadioHigh.setEnabled(checked);
        abRadioMedium.setEnabled(checked);
        abRadioLow.setEnabled(checked);
        abRadioAuto.setEnabled(checked);
        abRefresh.setEnabled(checked);
    }

    private void getColorManagerInstance() {

        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            abStatus = (TextView) rootView.findViewById(R.id.ab_text);
            abRefresh = (Button) rootView.findViewById(R.id.refresh_button);
            CheckBox enableAB = (CheckBox) rootView
                    .findViewById(R.id.ab_checkbox);
            abRadioHigh = (RadioButton) rootView
                    .findViewById(R.id.ab_high_radio);
            abRadioMedium = (RadioButton) rootView
                    .findViewById(R.id.ab_medium_radio);
            abRadioLow = (RadioButton) rootView.findViewById(R.id.ab_low_radio);
            abRadioAuto = (RadioButton) rootView
                    .findViewById(R.id.ab_auto_radio);
            abRadioGroup = (RadioGroup) rootView
                    .findViewById(R.id.ab_radioGroup);

            /* CB test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_ADAPTIVE_BACKLIGHT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_ab_toast,
                        Toast.LENGTH_LONG).show();
                setEnableUI(false);
                return;
            }

            // Initialize objects for backlight status
            abRefresh.setBackgroundResource(R.drawable.mode_style_user);
            abRefresh.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    updateBacklightStatus();
                }
            });

            // Initialize object for Checkbox UI element
            // Initialize initial state of checkbox
            enableAB.setChecked(cMgr.isAdaptiveBacklightEnabled());
            // Log.i(TAG,
            // "enableAB checkbox set to "
            // + cMgr.isAdaptiveBacklightEnabled());

            enableAB.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView,
                        boolean isChecked) {
                    final boolean checked = ((CheckBox) buttonView).isChecked();
                    // Log.i(TAG,
                    // "onCheckedChanged - AB enable checkbox clicked "
                    // + checked);
                    cMgr.setAdaptiveBacklightEnabled(checked);
                    if (checked) {
                        // set the back light level for this selection
                        int selectedABId = abRadioGroup
                                .getCheckedRadioButtonId();
                        RadioButton selectedABButton = (RadioButton) rootView
                                .findViewById(selectedABId);
                        if (null == selectedABButton) {
                            selectRadioButton();
                        } else {
                            setBacklightLvl(selectedABButton.getText()
                                    .toString());
                        }
                    }
                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            // Log.i(TAG,
                            // "Updating radio button group enable to "
                            // + checked);
                            setEnableUI(checked);
                            if (checked) {
                                updateBacklightStatus();
                            }
                        }
                    });
                }
            });
            // Initialize objects for Radio button UI elements
            // Log.i(TAG, "Initializing Radio button state");
            selectRadioButton(); // Initialize
                                 // initial
                                 // state
                                 // of the
                                 // radio
                                 // buttons
            abRadioGroup
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(RadioGroup group,
                                int checkedId) {
                            // Log.i(TAG, "onCheckedChanged for abRadioGroup");
                            // set the backlight level for this selection
                            RadioButton selectedABButton = (RadioButton) group
                                    .findViewById(checkedId);
                            setBacklightLvl(selectedABButton.getText()
                                    .toString());
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    updateBacklightStatus();
                                }
                            });
                        }
                    });

            if (!enableAB.isChecked()) {
                // Log.i(TAG,
                // "Initializing Radio buttons to false as enableAB is not checked");
                setEnableUI(false);
            } else {
                updateBacklightStatus();
            }
        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            Log.e(TAG, "Object creation failed");
        }
    }
}