/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file ColorBalanceSample.java
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
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_FEATURE;

public class CbFragment extends Fragment {

    private static String TAG = "DisplaySDK-CbFragment";
    private SeekBar cbControl = null;
    int cb = 0;
    ColorManager cmgr;
    private ImageView im;
    private Context appContext;
    private View rootView = null;

    /** Called when the activity is first created. */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.cb_sample, container, false);

        updateBackgroundImage();
        appContext = getActivity().getApplicationContext();

        if (null != cmgr) {
            // Log.i(TAG,
            // "onCreateView: Valid color manager object, initializing the UI");
            getColorManagerInstance();
        } else {
            Log.e(TAG, "onCreateView: null color manager object");
        }

        return rootView;
    }

    public void colorManagerInit(ColorManager mgr) {
        if (null != mgr) {
            cmgr = mgr;
            if (this.isVisible()) {
                // The color manager service just connected and this fragment is
                // visible initialize the UI
                // Log.i(TAG,
                // "colorManagerInit - visible fragment, initializing UI");
                getColorManagerInstance();
            }
        }
    }

    public void updateBackgroundImage() {
        if (null != rootView) {
            im = (ImageView) rootView.findViewById(R.id.surfaceimage);
            if (null != MenuActivity.bitmap) {
                im.setImageBitmap(MenuActivity.bitmap);
            }
        }
    }

    private void getColorManagerInstance() {
        // Log.i(TAG, "+getColorManagerInstance");
        if (cmgr != null) {
            boolean isSupport = false;

            cbControl = (SeekBar) rootView.findViewById(R.id.cb_bar);

            /* CB test */
            isSupport = cmgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_COLOR_BALANCE);
            if (isSupport == false) {
                Toast.makeText(appContext, R.string.disp_cb_toast,
                        Toast.LENGTH_LONG).show();
                cbControl.setEnabled(false);
                return;
            }
            cb = cmgr.getColorBalance();
            if (cb < ColorManager.COLOR_BALANCE_WARMTH_LOWER_BOUND
                    || cb > ColorManager.COLOR_BALANCE_WARMTH_UPPER_BOUND) {
                cb = 0;
            }
            // Log.i(TAG, "MAX warmth "
            // + ColorManager.COLOR_BALANCE_WARMTH_UPPER_BOUND
            // + " ; MIN warmth"
            // + ColorManager.COLOR_BALANCE_WARMTH_LOWER_BOUND);
            // Log.i(TAG, "Color balance value : " + cb);

            cmgr.setColorBalance(cb);
            String displayString = getResources().getString(
                    R.string.btn_color_balance);
            ((TextView) rootView.findViewById(R.id.textcb))
                    .setText(displayString + ": " + cb);

            cbControl.setMax(200);
            cbControl.setProgress(cb + 100);
            cbControl.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                    // Log.v(TAG, "setColorBalance returned " +
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onProgressChanged(SeekBar seekBar, int progress,
                        boolean fromUser) {
                    cb = progress - 100;
                    cmgr.setColorBalance(cb);
                    getActivity().runOnUiThread(new Runnable() {

                        @Override
                        public void run() {
                            String displayString = getResources().getString(
                                    R.string.btn_color_balance);
                            ((TextView) rootView.findViewById(R.id.textcb))
                                    .setText(displayString + ": " + cb);
                        }
                    });
                }
            });

        } else {
            Toast.makeText(appContext, R.string.disp_unsupported_toast,
                    Toast.LENGTH_LONG).show();
            Log.e(TAG, "Object creation failed");
        }
    }
}