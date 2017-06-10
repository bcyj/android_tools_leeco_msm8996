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

public class McSkinFragment extends Fragment {

    private static String TAG = "DisplaySDK-McSkinFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;

    private int mcskinhue = 0;
    private int mcskinsaturation = 0;
    private int mcskinintensity = 0;
    private SeekBar mcSkinControlHue = null;
    private SeekBar mcSkinControlSaturation = null;
    private SeekBar mcSkinControlIntensity = null;
    private CheckBox enableMcSkin = null;
    int HueMIN = 0, HueMAX = 0;
    int SaturationMIN = 0, SaturationMAX = 0;
    int IntensityMIN = 0, IntensityMAX = 0;

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.mcskin_sample, container, false);
        appContext = getActivity().getApplicationContext();
        updateBackgroundImage();

        if (null != cMgr) {
            // Log.i(TAG,
            // "onCreateView: Valid color manager object, initalizing the UI");
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
        mcSkinControlHue.setEnabled(checked);
        mcSkinControlSaturation.setEnabled(checked);
        mcSkinControlIntensity.setEnabled(checked);
    }

    private void getColorManagerInstance() {

        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            enableMcSkin = (CheckBox) rootView
                    .findViewById(R.id.mcskin_checkbox);
            mcSkinControlHue = (SeekBar) rootView.findViewById(R.id.mcskin_hue_bar);
            mcSkinControlSaturation = (SeekBar) rootView
                    .findViewById(R.id.mcskin_saturation_bar);
            mcSkinControlIntensity = (SeekBar) rootView
                    .findViewById(R.id.mcskin_intensity_bar);

            /* Memory Color test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_MEMORY_COLOR_ADJUSTMENT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_mcskin_toast,
                        Toast.LENGTH_LONG).show();
                enableMcSkin.setEnabled(false);
                setEnableUI(false);
                return;
            }

            // Initialize local variables
            MemoryColorConfig mcValues = cMgr
                    .getMemoryColorParams(MEMORY_COLOR_TYPE.SKIN);
            if (null != mcValues) {
                mcskinhue = mcValues.getHue();
                Log.i(TAG, "Initial Hue value is " + mcskinhue);
                mcskinsaturation = mcValues.getSaturation();
                Log.i(TAG, "Initial Saturation value is " + mcskinsaturation);
                mcskinintensity = mcValues.getIntensity();
                // Log.i(TAG, "Initial Intensity value is " + mcskinintensity);
            } else {
                Log.e(TAG, "getMemoryColorParams returned null during init");
            }

            setupHue();
            setupIntensity();
            setupSaturation();

            enableMcSkin
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            final boolean checked = ((CheckBox) buttonView)
                                    .isChecked();
                            // Log.i(TAG, "enableMcSkin onClick: - " + checked);
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    setEnableUI(checked);
                                    // Log.i(TAG,
                                    // "enableMcSkin onClick: updated UI elements - "
                                    // + checked);
                                }
                            });
                            if (checked) {
                                // Log.i(TAG,
                                // "enableMcSkin onClick: enable checked, enabling it in display");
                                setMcDisplayPrams();
                            } else {
                                // Log.i(TAG,
                                // "enableMcSkin setOnClickListener: enable unchecked, disabling it in display");
                                cMgr.disableMemoryColorConfig(MEMORY_COLOR_TYPE.SKIN);
                            }

                        }
                    });
            if (null != mcValues) {
                enableMcSkin.setChecked(mcValues.isMemoryColorEnabled());
                // Log.i(TAG,
                // "Setting  enableMcSkin checkbox to - "
                // + mcValues.isMemoryColorEnabled());
            }
            if (!enableMcSkin.isChecked()) {
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
        MemoryColorConfig newMcSkinConfig = new MemoryColorConfig(
                MEMORY_COLOR_TYPE.SKIN, mcskinhue, mcskinsaturation,
                mcskinintensity);
        cMgr.setMemoryColorParams(newMcSkinConfig);
    }

    private void setupHue() {
        // Log.i(TAG, "+setupHue");
        String displayStringHue = getResources().getString(R.string.mc_hue);
        ((TextView) rootView.findViewById(R.id.text_hue_mcskin))
                .setText(displayStringHue + ": " + mcskinhue);

        // Initialize local variables for various bounds
        HueMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.HUE);
        HueMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.HUE);
        // Log.i(TAG, "HueMAX " + HueMAX + " ; HueMIN " + HueMIN);

        mcSkinControlHue.setMax(HueMAX - HueMIN);
        mcSkinControlHue.setProgress(mcskinhue - HueMIN);
        mcSkinControlHue
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.i(TAG,
                        // "MC Skin Hue seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskinhue = progress + HueMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringHue = getResources()
                                        .getString(R.string.mc_hue);
                                ((TextView) rootView
                                        .findViewById(R.id.text_hue_mcskin))
                                        .setText(displayStringHue + ": "
                                                + mcskinhue);
                            }
                        });
                    }
                });
    }

    private void setupSaturation() {
        // Log.i(TAG, "+setupSaturation");
        String displayStringSaturation = getResources().getString(
                R.string.mc_saturation);
        ((TextView) rootView.findViewById(R.id.text_saturation_mcskin))
                .setText(displayStringSaturation + ": " + mcskinsaturation);

        // Initialize local variables for various bounds
        SaturationMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.SATURATION);
        SaturationMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.SATURATION);
        // Log.i(TAG, "SaturationMAX " + SaturationMAX + " ; SaturationMIN "
        // + SaturationMIN);

        mcSkinControlSaturation.setMax(SaturationMAX - SaturationMIN);
        mcSkinControlSaturation.setProgress(mcskinsaturation - SaturationMIN);
        mcSkinControlSaturation
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        Log.v(TAG,
                                "MC Skin Saturation seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskinsaturation = progress + SaturationMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringSaturation = getResources()
                                        .getString(R.string.mc_saturation);
                                ((TextView) rootView
                                        .findViewById(R.id.text_saturation_mcskin))
                                        .setText(displayStringSaturation + ": "
                                                + mcskinsaturation);
                            }
                        });
                    }
                });
    }

    private void setupIntensity() {
        // Log.i(TAG, "+setupIntensity");
        String displayStringIntensity = getResources().getString(
                R.string.mc_intensity);
        ((TextView) rootView.findViewById(R.id.text_intensity_mcskin))
                .setText(displayStringIntensity + ": " + mcskinintensity);

        // Initialize local variables for various bounds
        IntensityMAX = cMgr.getMaxLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.INTENSITY);
        IntensityMIN = cMgr.getMinLimitMemoryColor(MEMORY_COLOR_TYPE.SKIN,
                MEMORY_COLOR_PARAMS.INTENSITY);
        // Log.i(TAG, "IntensityMAX " + IntensityMAX + " ; IntensityMIN "
        // + IntensityMIN);

        mcSkinControlIntensity.setMax(IntensityMAX - IntensityMIN);
        mcSkinControlIntensity.setProgress(mcskinintensity - IntensityMIN);
        mcSkinControlIntensity
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        Log.v(TAG,
                                "MC Skin Intensity seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        mcskinintensity = progress + IntensityMIN;
                        setMcDisplayPrams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringIntensity = getResources()
                                        .getString(R.string.mc_intensity);
                                ((TextView) rootView
                                        .findViewById(R.id.text_intensity_mcskin))
                                        .setText(displayStringIntensity + ": "
                                                + mcskinintensity);
                            }
                        });
                    }
                });
    }
}