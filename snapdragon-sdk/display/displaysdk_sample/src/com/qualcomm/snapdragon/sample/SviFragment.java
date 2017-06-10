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

public class SviFragment extends Fragment {

    private static String TAG = "DisplaySDK-SviFragment";

    ColorManager cMgr = null;
    private View rootView = null;
    private Context appContext;

    private int svi = 0;
    private SeekBar sviControl = null;
    private CheckBox enableSVI = null;
    private int sviMAX = 0, sviMIN = 0;
    boolean sviInitalized = false;

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.svi_sample, container, false);
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
        // Sunlight Visibility is not saved as part of the mode so hide
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

    private void getColorManagerInstance() {
        // Log.i(TAG, "+getColorManagerInstance");
        if (cMgr != null) {
            boolean isSupport = false;

            sviControl = (SeekBar) rootView.findViewById(R.id.svi_bar);
            enableSVI = (CheckBox) rootView.findViewById(R.id.svi_checkbox);

            /* SVI test */
            isSupport = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_SUNLIGHT_VISBILITY_IMPROVEMENT);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_svi_toast,
                        Toast.LENGTH_LONG).show();
                sviControl.setEnabled(false);
                enableSVI.setEnabled(false);
                return;
            }

            String displayString = getResources().getString(R.string.svi_mode);
            ((TextView) rootView.findViewById(R.id.textsvi))
                    .setText(displayString + ": " + svi);

            enableSVI.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView,
                        boolean isChecked) {
                    final boolean checked = ((CheckBox) buttonView).isChecked();
                    cMgr.setSunlightVisibilityEnabled(checked);
                    if (checked && !sviInitalized) {
                        setupSVI();
                    }

                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            sviControl.setEnabled(checked);
                        }
                    });
                    // Log.i(TAG, "SVI enable checkbox clicked - " + checked);

                }
            });
            if (cMgr.isSunlightVisibilityEnabled()) {
                // set initial states of UI element
                enableSVI.setChecked(true);
                sviControl.setEnabled(true);
                setupSVI();
            } else {
                sviControl.setEnabled(false);
            }
        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            // Log.e(TAG, "Object creation failed");
        }
    }

    private void setupSVI() {
        svi = cMgr.getSunlightVisibilityStrength();
        sviMAX = cMgr.getMaxSunlightVisibilityStrength();
        sviMIN = cMgr.getMinSunlightVisibilityStrength();
        if (svi < sviMIN || svi > sviMAX) {
            svi = 0;
        }
        cMgr.setSunlightVisibilityStrength(svi);

        sviControl.setMax(sviMAX - sviMIN);
        sviControl.setProgress(svi - sviMIN);
        sviControl.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // Log.v(TAG, "SVI seek bar onStopTrackingTouch");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                svi = progress + sviMIN;
                cMgr.setSunlightVisibilityStrength(svi);
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        String displayString = getResources().getString(
                                R.string.svi_mode);
                        ((TextView) rootView.findViewById(R.id.textsvi))
                                .setText(displayString + ": " + svi);
                    }
                });
            }
        });

        sviInitalized = true;
    }
}