/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file SviSample.java
 *
 */
package com.qualcomm.snapdragon.sample;

import java.util.EnumSet;

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
import com.qti.snapdragon.sdk.display.PictureAdjustmentConfig;
import com.qti.snapdragon.sdk.display.PictureAdjustmentConfig.PICTURE_ADJUSTMENT_PARAMS;

public class PaFragment extends Fragment {

    private static String TAG = "DisplaySDK-PaFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;

    private int pasatthresh = 0;
    private int pahue = 0;
    private int pasaturation = 0;
    private int paintensity = 0;
    private int pacontrast = 0;
    private SeekBar paControlSatThresh = null;
    private SeekBar paControlHue = null;
    private SeekBar paControlSaturation = null;
    private SeekBar paControlIntensity = null;
    private SeekBar paControlContrast = null;
    private CheckBox enablePa = null;
    private CheckBox enablePaDesaturation = null;
    int SatThresholdMIN = 0, SatThresholdMAX = 0;
    int HueMIN = 0, HueMAX = 0;
    int SaturationMIN = 0, SaturationMAX = 0;
    int IntensityMIN = 0, IntensityMAX = 0;
    int ContrastMIN = 0, ContrastMAX = 0;

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.pa_sample, container, false);
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
        // Log.i(TAG, "colorManagerInit called");
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

    private void getColorManagerInstance() {
        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            enablePaDesaturation = (CheckBox) rootView
                    .findViewById(R.id.pa_desaturation_checkbox);
            enablePa = (CheckBox) rootView.findViewById(R.id.pa_checkbox);
            paControlSatThresh = (SeekBar) rootView
                    .findViewById(R.id.pa_sat_threshold_bar);
            paControlHue = (SeekBar) rootView.findViewById(R.id.pa_hue_bar);
            paControlSaturation = (SeekBar) rootView
                    .findViewById(R.id.pa_saturation_bar);
            paControlIntensity = (SeekBar) rootView
                    .findViewById(R.id.pa_intensity_bar);
            paControlContrast = (SeekBar) rootView
                    .findViewById(R.id.pa_contrast_bar);

            /* PA test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_GLOBAL_PICTURE_ADJUSTMENT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_pa_toast,
                        Toast.LENGTH_LONG).show();
                enablePa.setEnabled(false);
                enablePaDesaturation.setEnabled(false);
                paControlSatThresh.setEnabled(false);
                paControlHue.setEnabled(false);
                paControlSaturation.setEnabled(false);
                paControlIntensity.setEnabled(false);
                paControlContrast.setEnabled(false);
                return;
            }

            // Initialize local variables
            PictureAdjustmentConfig paValues = cMgr
                    .getPictureAdjustmentParams();
            if (null != paValues) {
                pahue = paValues.getHue();
                Log.i(TAG, "Initial Hue value is " + pahue);
                pasaturation = paValues.getSaturation();
                // Log.i(TAG, "Initial Saturation value is " + pasaturation);
                paintensity = paValues.getIntensity();
                // Log.i(TAG, "Initial Intensity value is " + paintensity);
                pasatthresh = paValues.getSaturationThreshold();
                // Log.i(TAG, "Initial Saturation Threshold value is "
                // + pasatthresh);
                pacontrast = paValues.getContrast();
                // Log.i(TAG, "Initial Contrast value is " + pacontrast);
            } else {
                Log.e(TAG,
                        "getPictureAdjustmentParams returned null during init");
            }

            setupSatThreshold();
            setupHue();
            setupIntensity();
            setupSaturation();
            setupContrast();

            enablePaDesaturation
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            final boolean checked = ((CheckBox) buttonView)
                                    .isChecked();
                            // Log.i(TAG,
                            // "enablePaDesaturation onClick: enable PA desaturation clicked - "
                            // + checked);
                            // This function can be triggered only when
                            // pa_checkbox is
                            // checked first
                            // otherwise pa_desaturaiton_checkbox will be
                            // disabled and
                            // will not generate
                            // this event so manipulate the UI elements without
                            // needing
                            // to check global enable
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    // if the desaturation is enabled then
                                    // disable the
                                    // controls
                                    // and vice versa
                                    paControlSatThresh.setEnabled(!checked);
                                    paControlHue.setEnabled(!checked);
                                    paControlSaturation.setEnabled(!checked);
                                    paControlIntensity.setEnabled(!checked);
                                    paControlContrast.setEnabled(!checked);
                                    // Log.i(TAG,
                                    // "enablePaDesaturation onClick: updated UI elements - "
                                    // + checked);
                                }
                            });
                            if (checked) {
                                // Log.i(TAG,
                                // "enabled Picture Adjustment in display");
                                cMgr.enablePictureAdjustmentDesaturation();
                            } else {
                                setPaDisplayParams();
                            }

                        }
                    });
            if (null != paValues) {
                enablePaDesaturation.setChecked(paValues
                        .isDesaturationEnabled());
                // Log.i(TAG, "Setting  enablePaDesaturation checkbox to - "
                // + paValues.isDesaturationEnabled());
            }

            enablePa.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton buttonView,
                        boolean isChecked) {
                    final boolean checked = ((CheckBox) buttonView).isChecked();
                    // Log.i(TAG, "enablePa onClick: enable PA clicked - "
                    // + checked);
                    // if desaturation is not enabled then enable all the
                    // control seek bars
                    final boolean paControlEnable = checked
                            && !(enablePaDesaturation.isChecked());
                    if (checked) {
                        // The Picture adjustment just got enabled, so enable
                        // the feature
                        // Log.i(TAG,
                        // "enablePa onClick: enable PA checked, enabling it in display");
                        setPaDisplayParams();
                        if (enablePaDesaturation.isChecked()) {
                            // enable desaturation if checked
                            // Log.i(TAG,
                            // "enablePa setOnClickListener: enablePaDesaturation is checked, enabling desaturation in display");
                            cMgr.enablePictureAdjustmentDesaturation();
                        }
                    } else {
                        // Log.i(TAG,
                        // "enablePa setOnClickListener: enable PA unchecked, disabling it in display");
                        cMgr.disablePictureAdjustmentConfig();
                    }
                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            // Log.i(TAG,
                            // "enablePa onCheckedChanged - desaturation checkbox enabled to "
                            // + checked);
                            // Log.i(TAG,
                            // "enablePa onCheckedChanged - pa seek bars enabled to "
                            // + paControlEnable);
                            enablePaDesaturation.setEnabled(checked);
                            paControlSatThresh.setEnabled(paControlEnable);
                            paControlHue.setEnabled(paControlEnable);
                            paControlSaturation.setEnabled(paControlEnable);
                            paControlIntensity.setEnabled(paControlEnable);
                            paControlContrast.setEnabled(paControlEnable);
                        }
                    });

                }
            });

            if (null != paValues) {
                enablePa.setChecked(!paValues
                        .isGlobalPictureAdjustmentDisabled());
            }

            if (!enablePa.isChecked() || enablePaDesaturation.isChecked()) {
                // Log.i(TAG,
                // "getColorManagerInstance: Disabling all sliding controls");
                paControlSatThresh.setEnabled(false);
                paControlHue.setEnabled(false);
                paControlSaturation.setEnabled(false);
                paControlIntensity.setEnabled(false);
                paControlContrast.setEnabled(false);
            }
            if (!enablePa.isChecked()) {
                // Log.i(TAG,
                // "getColorManagerInstance: Disabling desaturation checkbox");
                enablePaDesaturation.setEnabled(false);
            }

        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            Log.e(TAG, "Object creation failed");
        }
    }

    private void setPaDisplayParams() {
        // Log.i(TAG, "+setPaDisplayParams - setting params to display");
        PictureAdjustmentConfig newPaConfig = new PictureAdjustmentConfig(
                EnumSet.allOf(PictureAdjustmentConfig.PICTURE_ADJUSTMENT_PARAMS.class),
                pahue, pasaturation, paintensity, pacontrast, pasatthresh);
        cMgr.setPictureAdjustmentParams(newPaConfig);
    }

    private void setupSatThreshold() {
        // Log.i(TAG, "+setupSatThreshold");
        String displayStringSatThresh = getResources().getString(
                R.string.pa_sat_thresh);
        ((TextView) rootView.findViewById(R.id.text_sat_threshold_pa))
                .setText(displayStringSatThresh + ": " + pasatthresh);

        // Initialize local variables for various bounds
        SatThresholdMAX = cMgr
                .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION_THRESHOLD);
        SatThresholdMIN = cMgr
                .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION_THRESHOLD);

        // Log.i(TAG, "SatThresholdMAX " + SatThresholdMAX +
        // " ; SatThresholdMIN "
        // + SatThresholdMIN);

        paControlSatThresh.setMax(SatThresholdMAX - SatThresholdMIN);
        paControlSatThresh.setProgress(pasatthresh - SatThresholdMIN);
        paControlSatThresh
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.v(TAG,
                        // "PA Saturation Threshold seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        pasatthresh = progress + SatThresholdMIN;
                        setPaDisplayParams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringSatThresh = getResources()
                                        .getString(R.string.pa_sat_thresh);
                                ((TextView) rootView
                                        .findViewById(R.id.text_sat_threshold_pa))
                                        .setText(displayStringSatThresh + ": "
                                                + pasatthresh);
                            }
                        });
                    }
                });
    }

    private void setupHue() {
        // Log.i(TAG, "+setupHue");
        String displayStringHue = getResources().getString(R.string.pa_hue);
        ((TextView) rootView.findViewById(R.id.text_hue_pa))
                .setText(displayStringHue + ": " + pahue);

        // Initialize local variables for various bounds
        HueMAX = cMgr
                .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.HUE);
        HueMIN = cMgr
                .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.HUE);
        // Log.i(TAG, "HueMAX " + HueMAX + " ; HueMIN " + HueMIN);

        paControlHue.setMax(HueMAX - HueMIN);
        paControlHue.setProgress(pahue - HueMIN);
        paControlHue.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // Log.v(TAG, "PA Hue seek bar onStopTrackingTouch");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                pahue = progress + HueMIN;
                setPaDisplayParams();
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        String displayStringHue = getResources().getString(
                                R.string.pa_hue);
                        ((TextView) rootView.findViewById(R.id.text_hue_pa))
                                .setText(displayStringHue + ": " + pahue);
                    }
                });
            }
        });
    }

    private void setupSaturation() {
        // Log.i(TAG, "+setupSaturation");
        String displayStringSaturation = getResources().getString(
                R.string.pa_saturation);
        ((TextView) rootView.findViewById(R.id.text_saturation_pa))
                .setText(displayStringSaturation + ": " + pasaturation);

        // Initialize local variables for various bounds
        SaturationMAX = cMgr
                .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION);
        SaturationMIN = cMgr
                .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION);
        // Log.i(TAG, "SaturationMAX " + SaturationMAX + " ; SaturationMIN "
        // + SaturationMIN);

        paControlSaturation.setMax(SaturationMAX - SaturationMIN);
        paControlSaturation.setProgress(pasaturation - SaturationMIN);
        paControlSaturation
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.v(TAG,
                        // "PA Saturation seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        pasaturation = progress + SaturationMIN;
                        setPaDisplayParams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringSaturation = getResources()
                                        .getString(R.string.pa_saturation);
                                ((TextView) rootView
                                        .findViewById(R.id.text_saturation_pa))
                                        .setText(displayStringSaturation + ": "
                                                + pasaturation);
                            }
                        });
                    }
                });
    }

    private void setupIntensity() {
        // Log.i(TAG, "+setupIntensity");
        String displayStringIntensity = getResources().getString(
                R.string.pa_intensity);
        ((TextView) rootView.findViewById(R.id.text_intensity_pa))
                .setText(displayStringIntensity + ": " + paintensity);

        // Initialize local variables for various bounds
        IntensityMAX = cMgr
                .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.INTENSITY);
        IntensityMIN = cMgr
                .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.INTENSITY);
        // Log.i(TAG, "IntensityMAX " + IntensityMAX + " ; IntensityMIN "
        // + IntensityMIN);

        paControlIntensity.setMax(IntensityMAX - IntensityMIN);
        paControlIntensity.setProgress(paintensity - IntensityMIN);
        paControlIntensity
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.i(TAG,
                        // "Pa Intensity seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        paintensity = progress + IntensityMIN;
                        setPaDisplayParams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringIntensity = getResources()
                                        .getString(R.string.pa_intensity);
                                ((TextView) rootView
                                        .findViewById(R.id.text_intensity_pa))
                                        .setText(displayStringIntensity + ": "
                                                + paintensity);
                            }
                        });
                    }
                });
    }

    private void setupContrast() {
        // Log.i(TAG, "+setupContrast");
        String displayStringContrast = getResources().getString(
                R.string.pa_contrast);
        ((TextView) rootView.findViewById(R.id.text_contrast_pa))
                .setText(displayStringContrast + ": " + pacontrast);

        // Initialize local variables for various bounds
        ContrastMAX = cMgr
                .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.CONTRAST);
        ContrastMIN = cMgr
                .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.CONTRAST);

        // Log.i(TAG, "ContrastMAX " + ContrastMAX + " ; ContrastMIN "
        // + ContrastMIN);

        paControlContrast.setMax(ContrastMAX - ContrastMIN);
        paControlContrast.setProgress(pacontrast - ContrastMIN);
        paControlContrast
                .setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        // Log.v(TAG,
                        // "Pa Contrast seek bar onStopTrackingTouch");
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onProgressChanged(SeekBar seekBar,
                            int progress, boolean fromUser) {
                        pacontrast = progress + ContrastMIN;
                        setPaDisplayParams();
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                String displayStringContrast = getResources()
                                        .getString(R.string.pa_contrast);
                                ((TextView) rootView
                                        .findViewById(R.id.text_contrast_pa))
                                        .setText(displayStringContrast + ": "
                                                + pacontrast);
                            }
                        });
                    }
                });
    }

}