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
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_FEATURE;
import com.qti.snapdragon.sdk.display.MemoryColorConfig;
import com.qti.snapdragon.sdk.display.MemoryColorConfig.MEMORY_COLOR_PARAMS;
import com.qti.snapdragon.sdk.display.MemoryColorConfig.MEMORY_COLOR_TYPE;

public class McSkyFragment extends Fragment {

    private static String TAG = "DisplaySDK-McSkyFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;

    private int mcskyhue = 0;
    private int mcskysaturation = 0;
    private int mcskyintensity = 0;
    private SeekBar mcSkyControlHue = null;
    private SeekBar mcSkyControlSaturation = null;
    private SeekBar mcSkyControlIntensity = null;
    private CheckBox enableMcSky = null;
    int HueMIN = 0, HueMAX = 0;
    int SaturationMIN = 0, SaturationMAX = 0;
    int IntensityMIN = 0, IntensityMAX = 0;

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.mcsky_sample, container, false);
        appContext = getActivity().getApplicationContext();
        updateBackgroundImage();

        if (null != cMgr) {
            // Log.i(TAG,
            // "onCreateView: Valid color manager object, initializing the UI");
            getColorManagerInstance();
        } else {
            Log.e(TAG, "onCreateView: null color manager object");
        }

        return rootView;
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
                // visible, initialize the UI
                // Log.i(TAG,
                // "colorManagerInit - visible fragment, initializing UI");
                getColorManagerInstance();
            }
        }
    }

private void setEnableUI(boolean checked){
    mcSkyControlHue.setEnabled(checked);
    mcSkyControlSaturation.setEnabled(checked);
    mcSkyControlIntensity.setEnabled(checked);
}

private void getColorManagerInstance() {
        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            enableMcSky = (CheckBox) rootView.findViewById(R.id.mcsky_checkbox);
            mcSkyControlHue = (SeekBar) rootView.findViewById(R.id.mcsky_hue_bar);
            mcSkyControlSaturation = (SeekBar) rootView
                    .findViewById(R.id.mcsky_saturation_bar);
            mcSkyControlIntensity = (SeekBar) rootView
                    .findViewById(R.id.mcsky_intensity_bar);

            /* MC test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_MEMORY_COLOR_ADJUSTMENT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_mcsky_toast,
                        Toast.LENGTH_LONG).show();
                enableMcSky.setEnabled(false);
                setEnableUI(false);
                return;
            }

            // Initialize local variables
            MemoryColorConfig mcValues = cMgr
                    .getMemoryColorParams(MEMORY_COLOR_TYPE.SKY);
            if (null != mcValues) {
                mcskyhue = mcValues.getHue();
                Log.i(TAG, "Initial Hue value is " + mcskyhue);
                mcskysaturation = mcValues.getSaturation();
                Log.i(TAG, "Initial Saturation value is " + mcskysaturation);
                mcskyintensity = mcValues.getIntensity();
                Log.i(TAG, "Initial Intensity value is " + mcskyintensity);
            } else {
                Log.e(TAG, "getMemoryColorParams returned null during init");
            }

            setupHue();
            setupIntensity();
            setupSaturation();

            enableMcSky
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            final boolean checked = ((CheckBox) buttonView)
                                    .isChecked();
                            // Log.i(TAG, "enableMcSky onCheckedChanged: - "
                            // + checked);
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    setEnableUI(checked);
                                    // Log.i(TAG,
                                    // "enableMcSky onCheckedChanged: updated UI elements - "
                                    // + checked);
                                }
                            });
                            if (checked) {
                                // Log.i(TAG,
                                // "enableMcSky onCheckedChanged: enable checked, enabling it in display");
                                setMcDisplayPrams();
                            } else {
                                // Log.i(TAG,
                                // "enableMcSky onCheckedChanged: enable unchecked, disabling it in display");
                                cMgr.disableMemoryColorConfig(MEMORY_COLOR_TYPE.SKY);
                            }

                        }
                    });
            if (null != mcValues) {
                enableMcSky.setChecked(mcValues.isMemoryColorEnabled());
                // Log.i(TAG,
                // "Setting  enableMcSky checkbox to - "
                // + mcValues.isMemoryColorEnabled());
            }
            if (!enableMcSky.isChecked()) {
                // Log.i(TAG,
                // "getColorManagerInstance: Disabling all sliding controls");
                setEnableUI(false);
            }

        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            Log.e(TAG, "Object creation failed");
        }
    }

    private void setMcDisplayPrams() {
        // Log.i(TAG, "+setMcDisplayPrams");
        MemoryColorConfig newMcSkyConfig = new MemoryColorConfig(
                MEMORY_COLOR_TYPE.SKY, mcskyhue, mcskysaturation,
                mcskyintensity);
        cMgr.setMemoryColorParams(newMcSkyConfig);
    }

    private void setupHue() {
        // Log.i(TAG, "+setupHue");
        String displayStringHue = getResources().getString(R.string.mc_hue);
        ((TextView) rootView.findViewById(R.id.text_hue_mcsky))
                .setText(displayStringHue + ": " + mcskyhue);

        // Initialize local variables for various bounds
        HueMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.HUE);
        HueMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.HUE);
        // Log.i(TAG, "HueMAX " + HueMAX + " ; HueMIN " + HueMIN);

        mcSkyControlHue.setMax(HueMAX - HueMIN);
        mcSkyControlHue.setProgress(mcskyhue - HueMIN);
        mcSkyControlHue
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.i(TAG,
                        // "MC Sky Hue seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskyhue = progress + HueMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringHue = getResources()
                                        .getString(R.string.mc_hue);
                                ((TextView) rootView
                                        .findViewById(R.id.text_hue_mcsky))
                                        .setText(displayStringHue + ": "
                                                + mcskyhue);
                            }
                        });
                    }
                });
    }

    private void setupSaturation() {
        // Log.i(TAG, "+setupSaturation");
        String displayStringSaturation = getResources().getString(
                R.string.mc_saturation);
        ((TextView) rootView.findViewById(R.id.text_saturation_mcsky))
                .setText(displayStringSaturation + ": " + mcskysaturation);

        // Initialize local variables for various bounds
        SaturationMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKY,
                MEMORY_COLOR_PARAMS.SATURATION);
        SaturationMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKY,
                MEMORY_COLOR_PARAMS.SATURATION);
        // Log.i(TAG, "SaturationMAX " + SaturationMAX + " ; SaturationMIN "
        // + SaturationMIN);

        mcSkyControlSaturation.setMax(SaturationMAX - SaturationMIN);
        mcSkyControlSaturation.setProgress(mcskysaturation - SaturationMIN);
        mcSkyControlSaturation
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.v(TAG,
                        // "MC Sky Saturation seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskysaturation = progress + SaturationMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringSaturation = getResources()
                                        .getString(R.string.mc_saturation);
                                ((TextView) rootView
                                        .findViewById(R.id.text_saturation_mcsky))
                                        .setText(displayStringSaturation + ": "
                                                + mcskysaturation);
                            }
                        });
                    }
                });
    }

    private void setupIntensity() {
        // Log.i(TAG, "+setupIntensity");
        String displayStringIntensity = getResources().getString(
                R.string.mc_intensity);
        ((TextView) rootView.findViewById(R.id.text_intensity_mcsky))
                .setText(displayStringIntensity + ": " + mcskyintensity);

        // Initialize local variables for various bounds
        IntensityMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.INTENSITY);
        IntensityMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.INTENSITY);
        // Log.i(TAG, "IntensityMAX " + IntensityMAX + " ; IntensityMIN "
        // + IntensityMIN);

        mcSkyControlIntensity.setMax(IntensityMAX - IntensityMIN);
        mcSkyControlIntensity.setProgress(mcskyintensity - IntensityMIN);
        mcSkyControlIntensity
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.v(TAG,
                        // "MC Sky Intensity seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskyintensity = progress + IntensityMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringIntensity = getResources()
                                        .getString(R.string.mc_intensity);
                                ((TextView) rootView
                                        .findViewById(R.id.text_intensity_mcsky))
                                        .setText(displayStringIntensity + ": "
                                                + mcskyintensity);
                            }
                        });
                    }
                });
    }
}