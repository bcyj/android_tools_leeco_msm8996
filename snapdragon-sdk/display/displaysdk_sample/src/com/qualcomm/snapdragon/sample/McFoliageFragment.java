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

public class McFoliageFragment extends Fragment {

    private static String TAG = "DisplaySDK-McFoliageFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;

    private int mcfoliagehue = 0;
    private int mcfoliagesaturation = 0;
    private int mcfoliageintensity = 0;
    private SeekBar mcFoliageControlHue = null;
    private SeekBar mcFoliageControlSaturation = null;
    private SeekBar mcFoliageControlIntensity = null;
    private CheckBox enableMcFoliage = null;
    int HueMIN = 0, HueMAX = 0;
    int SaturationMIN = 0, SaturationMAX = 0;
    int IntensityMIN = 0, IntensityMAX = 0;

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater
                .inflate(R.layout.mcfoliage_sample, container, false);
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
                // visible initialize the UI
                // Log.i(TAG,
                // "colorManagerInit - visible fragment, initializing UI");
                getColorManagerInstance();
            }
        }
    }

    private void setEnableUI(boolean checked){
        mcFoliageControlHue.setEnabled(checked);
        mcFoliageControlSaturation.setEnabled(checked);
        mcFoliageControlIntensity.setEnabled(checked);
    }

    private void getColorManagerInstance() {

        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            enableMcFoliage = (CheckBox) rootView
                    .findViewById(R.id.mcfoliage_checkbox);
            mcFoliageControlHue = (SeekBar) rootView
                    .findViewById(R.id.mcfoliage_hue_bar);
            mcFoliageControlSaturation = (SeekBar) rootView
                    .findViewById(R.id.mcfoliage_saturation_bar);
            mcFoliageControlIntensity = (SeekBar) rootView
                    .findViewById(R.id.mcfoliage_intensity_bar);

            /* Memory Color test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_MEMORY_COLOR_ADJUSTMENT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_mcfoliage_toast,
                        Toast.LENGTH_LONG).show();
                enableMcFoliage.setEnabled(false);
                setEnableUI(false);
                return;
            }

            // Initialize local variables
            MemoryColorConfig mcValues = cMgr
                    .getMemoryColorParams(MEMORY_COLOR_TYPE.FOLIAGE);
            if (null != mcValues) {
                mcfoliagehue = mcValues.getHue();
                // Log.i(TAG, "Initial Hue value is " + mcfoliagehue);
                mcfoliagesaturation = mcValues.getSaturation();
                // Log.i(TAG, "Initial Saturation value is " +
                // mcfoliagesaturation);
                mcfoliageintensity = mcValues.getIntensity();
                // Log.i(TAG, "Initial Intensity value is " +
                // mcfoliageintensity);
            } else {
                Log.e(TAG, "getMemoryColorParams returned null during init");
            }

            setupHue();
            setupIntensity();
            setupSaturation();

            enableMcFoliage
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            final boolean checked = ((CheckBox) buttonView)
                                    .isChecked();
                            // Log.i(TAG,
                            // "enableMcFoliage setOnCheckedChangeListener: - "
                            // + checked);
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    setEnableUI(checked);
                                    // Log.i(TAG,
                                    // "enableMcFoliage onClick: updated UI elements - "
                                    // + checked);
                                }
                            });
                            if (checked) {
                                // Log.i(TAG,
                                // "enableMcFoliage setOnCheckedChangeListener: enable checked, enabling it in display");
                                setMcDisplayPrams();
                            } else {
                                // Log.i(TAG,
                                // "enableMcFoliage setOnCheckedChangeListener: enable unchecked, disabling it in display");
                                cMgr.disableMemoryColorConfig(MEMORY_COLOR_TYPE.FOLIAGE);
                            }
                        }
                    });
            if (null != mcValues) {
                enableMcFoliage.setChecked(mcValues.isMemoryColorEnabled());
                // Log.i(TAG,
                // "Setting  enableMcFoliage checkbox to - "
                // + mcValues.isMemoryColorEnabled());
            }
            // Log.i(TAG,
            // "getColorManagerInstance: updating sliding controls to "
            // + enableMcFoliage.isChecked());
            setEnableUI(enableMcFoliage.isChecked());
        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            Log.e(TAG, "Object creation failed");
        }
    }

    private void setMcDisplayPrams() {
        // Log.i(TAG, "+setMcDisplayPrams");
        MemoryColorConfig newMcFoliageConfig = new MemoryColorConfig(
                MEMORY_COLOR_TYPE.FOLIAGE, mcfoliagehue, mcfoliagesaturation,
                mcfoliageintensity);
        cMgr.setMemoryColorParams(newMcFoliageConfig);
    }

    private void setupHue() {
        // Log.i(TAG, "+setupHue");
        String displayStringHue = getResources().getString(R.string.mc_hue);
        ((TextView) rootView.findViewById(R.id.text_hue_mcfoliage))
                .setText(displayStringHue + ": " + mcfoliagehue);

        // Initialize local variables for various bounds
        HueMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.HUE);
        HueMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.HUE);
        // Log.i(TAG, "HueMAX " + HueMAX + " ; HueMIN " + HueMIN);

        mcFoliageControlHue.setMax(HueMAX - HueMIN);
        mcFoliageControlHue.setProgress(mcfoliagehue - HueMIN);
        mcFoliageControlHue
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        Log.v(TAG,
                                "MC Foliage Hue seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcfoliagehue = progress + HueMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringHue = getResources()
                                        .getString(R.string.mc_hue);
                                ((TextView) rootView
                                        .findViewById(R.id.text_hue_mcfoliage))
                                        .setText(displayStringHue + ": "
                                                + mcfoliagehue);
                            }
                        });
                    }
                });
    }

    private void setupSaturation() {
        // Log.i(TAG, "+setupSaturation");
        String displayStringSaturation = getResources().getString(
                R.string.mc_saturation);
        ((TextView) rootView.findViewById(R.id.text_saturation_mcfoliage))
                .setText(displayStringSaturation + ": " + mcfoliagesaturation);

        // Initialize local variables for various bounds
        SaturationMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.SATURATION);
        SaturationMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.SATURATION);
        // Log.i(TAG, "SaturationMAX " + SaturationMAX + " ; SaturationMIN "
        // + SaturationMIN);

        mcFoliageControlSaturation.setMax(SaturationMAX - SaturationMIN);
        mcFoliageControlSaturation.setProgress(mcfoliagesaturation
                - SaturationMIN);
        mcFoliageControlSaturation
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        Log.v(TAG,
                                "MC Foliage Saturation seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcfoliagesaturation = progress + SaturationMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringSaturation = getResources()
                                        .getString(R.string.mc_saturation);
                                ((TextView) rootView
                                        .findViewById(R.id.text_saturation_mcfoliage))
                                        .setText(displayStringSaturation + ": "
                                                + mcfoliagesaturation);
                            }
                        });
                    }
                });
    }

    private void setupIntensity() {
        // Log.i(TAG, "+setupIntensity");
        String displayStringIntensity = getResources().getString(
                R.string.mc_intensity);
        ((TextView) rootView.findViewById(R.id.text_intensity_mcfoliage))
                .setText(displayStringIntensity + ": " + mcfoliageintensity);

        // Initialize local variables for various bounds
        IntensityMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.INTENSITY);
        IntensityMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.FOLIAGE,
                MEMORY_COLOR_PARAMS.INTENSITY);
        // Log.i(TAG, "IntensityMAX " + IntensityMAX + " ; IntensityMIN "
        // + IntensityMIN);

        mcFoliageControlIntensity.setMax(IntensityMAX - IntensityMIN);
        mcFoliageControlIntensity
                .setProgress(mcfoliageintensity - IntensityMIN);
        mcFoliageControlIntensity
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        Log.v(TAG,
                                "MC Foliage Intensity seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcfoliageintensity = progress + IntensityMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringIntensity = getResources()
                                        .getString(R.string.mc_intensity);
                                ((TextView) rootView
                                        .findViewById(R.id.text_intensity_mcfoliage))
                                        .setText(displayStringIntensity + ": "
                                                + mcfoliageintensity);
                            }
                        });
                    }
                });
    }
}
