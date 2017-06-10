/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import com.qti.ultrasound.penpairing.customviews.BarChartView;
import com.qti.ultrasound.penpairing.customviews.BarChartView.BarConstants;

import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.TreeMap;

public class TesterActivity extends SWCalibrationActivity {

    private static final String SW_CALIB_CFG_PATH = "/data/usf/sw_calib/usf_sw_calib.cfg";

    private static final String SW_CALIB_TESTER_CFG_PATH = "/data/usf/sw_calib/usf_sw_calib_tester.cfg";

    private static final int MSG_IND_TYPE = 0;

    private static final int MSG_IND_MIC = 1;

    private static final int MSG_IND_VALUE = 2;

    /*
     * MSG_TYPE constants. These values MUST match the ones
     * defined in the sw_calib daemon.
     */
    private static final int MSG_TYPE_MIN_SCALE_POWER = 0;

    private static final int MSG_TYPE_MAX_SCALE_POWER = 1;

    private static final int MSG_TYPE_THRESHOLD_POWER = 2;

    private static final int MSG_TYPE_MIN_SCALE_QUALITY = 3;

    private static final int MSG_TYPE_MAX_SCALE_QUALITY = 4;

    private static final int MSG_TYPE_THRESHOLD_QUALITY = 5;

    private static final int MSG_TYPE_MEASUREMENT_POWER = 6;

    private static final int MSG_TYPE_MEASUREMENT_QUALITY = 7;

    private static final int MSG_TYPE_INTERFERENCE = 8;

    private static final int TESTER_CONTROL_THREAD_TIMEOUT_MSEC = Integer.MAX_VALUE; // no timeout

    private BarChartView mSignalPowerChart = null;

    private TreeMap<Integer, BarConstants> mSignalPowerConstantsList = new TreeMap<Integer, BarConstants>();

    private TreeMap<Integer, Float> mSignalPowerMeasurementBuffer = new TreeMap<Integer, Float>();

    private BarChartView mSignalQualityChart = null;

    private TreeMap<Integer, BarConstants> mSignalQualityConstantsList = new TreeMap<Integer, BarConstants>();

    private TreeMap<Integer, Float> mSignalQualityMeasurementBuffer = new TreeMap<Integer, Float>();

    private String mOriginalCfgPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tester);
    }

    @Override
    protected void onStop() {
        restoreOriginalCfgLink();
        super.onStop();
    }

    @Override
    protected int getTimeoutForControlThread() {
        return TESTER_CONTROL_THREAD_TIMEOUT_MSEC;
    }

    private void restoreOriginalCfgLink() {
        try {
            Runtime.getRuntime().exec("rm -f " + SW_CALIB_CFG_PATH);
            Runtime.getRuntime().exec(
                    "ln -s " + mOriginalCfgPath + " " + SW_CALIB_CFG_PATH);
        } catch (IOException e) {
            Log.e(this.toString(), e.toString());
            alertAndQuit("Software Calibration Failed", "Failed restoring original cfg.");
        }
    }

    @Override
    protected void onPairingSuccessful(int penId) {
        setLinkToTesterCfg();
        super.onPairingSuccessful(penId);
        setOnScreenText("Pen detected (series ID " + String.valueOf(penId) + ")",
                R.id.sw_calib_text_view);
    }

    @Override
    protected void onSwCalibSocketEvent(int[] res) {
        int type = res[MSG_IND_TYPE];
        int mic = res[MSG_IND_MIC];
        float val = Float.intBitsToFloat(res[MSG_IND_VALUE]);

        Log.d(this.toString(), "Got on socket: " + type + " " + mic + " " + String.valueOf(val));

        if (!mSignalPowerConstantsList.containsKey(mic)) {
            mSignalPowerConstantsList.put(mic, new BarConstants(String.valueOf(mic)));
            mSignalQualityConstantsList.put(mic, new BarConstants(String.valueOf(mic)));
        }

        setOnScreenText("Testing in progress...", R.id.sw_calib_text_view);
        setCircleColor(Color.GREEN, findViewById(R.id.sw_calib_circle_view));

        switch (type) {
            case MSG_TYPE_MEASUREMENT_POWER:
                if (mSignalPowerChart == null) {
                    mSignalPowerChart = new BarChartView(this,
                            mSignalPowerConstantsList.values().toArray(new BarConstants[mSignalPowerConstantsList.size()]),
                            "Signal Power (dB)");
                    RelativeLayout.LayoutParams rlp_left = new RelativeLayout.LayoutParams(
                            LayoutParams.WRAP_CONTENT, LayoutParams.MATCH_PARENT);
                    rlp_left.addRule(RelativeLayout.LEFT_OF, R.id.sw_calib_text_view);
                    mSignalPowerChart.setBackgroundColor(Color.WHITE);
                    ((RelativeLayout)findViewById(R.id.tester_root_view)).addView(mSignalPowerChart, rlp_left);
                }
                mSignalPowerMeasurementBuffer.put(mic, val);
                if (mSignalPowerMeasurementBuffer.size() == mSignalPowerConstantsList.size()) {
                    // we have values for all mics - update chart
                    mSignalPowerChart.postNewChartData(mSignalPowerMeasurementBuffer
                            .values().toArray(new Float[mSignalPowerMeasurementBuffer.size()]));
                    mSignalPowerMeasurementBuffer.clear();
                }
                break;
            case MSG_TYPE_MEASUREMENT_QUALITY:
                if (mSignalQualityChart == null) {
                    mSignalQualityChart = new BarChartView(this,
                            mSignalQualityConstantsList.values().toArray(new BarConstants[mSignalQualityConstantsList.size()]),
                            "Signal Quality (peak/RMS)");
                    RelativeLayout.LayoutParams rlp_right = new RelativeLayout.LayoutParams(
                            LayoutParams.WRAP_CONTENT, LayoutParams.MATCH_PARENT);
                    rlp_right.addRule(RelativeLayout.RIGHT_OF, R.id.sw_calib_text_view);
                    mSignalQualityChart.setBackgroundColor(Color.WHITE);

                    ((RelativeLayout)findViewById(R.id.tester_root_view)).addView(mSignalQualityChart,
                            rlp_right);
                }
                mSignalQualityMeasurementBuffer.put(mic, val);
                if (mSignalQualityMeasurementBuffer.size() == mSignalQualityConstantsList.size()) {
                    // we have values for all mics - update chart
                    mSignalQualityChart.postNewChartData(mSignalQualityMeasurementBuffer
                            .values().toArray(new Float[mSignalQualityMeasurementBuffer.size()]));
                    mSignalQualityMeasurementBuffer.clear();
                }
                break;
            case MSG_TYPE_MIN_SCALE_POWER:
                mSignalPowerConstantsList.get(mic).mScaleMin = val;
                break;
            case MSG_TYPE_MAX_SCALE_POWER:
                mSignalPowerConstantsList.get(mic).mScaleMax = val;
                break;
            case MSG_TYPE_THRESHOLD_POWER:
                mSignalPowerConstantsList.get(mic).mThreshold = val;
                break;
            case MSG_TYPE_MIN_SCALE_QUALITY:
                mSignalQualityConstantsList.get(mic).mScaleMin = val;
                break;
            case MSG_TYPE_MAX_SCALE_QUALITY:
                mSignalQualityConstantsList.get(mic).mScaleMax = val;
                break;
            case MSG_TYPE_THRESHOLD_QUALITY:
                mSignalQualityConstantsList.get(mic).mThreshold = val;
                break;
            case MSG_TYPE_INTERFERENCE:
                setOnScreenText("Interference detected!", R.id.sw_calib_text_view);
                setCircleColor(Color.RED, findViewById(R.id.sw_calib_circle_view));
                break;
        }
    }

    private void setLinkToTesterCfg() {
        try {
            Process p = Runtime.getRuntime().exec("readlink " + SW_CALIB_CFG_PATH);
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(
                    p.getInputStream()));
            mOriginalCfgPath = bufferedReader.readLine();
            Runtime.getRuntime().exec("rm -f " + SW_CALIB_CFG_PATH);
            Runtime.getRuntime().exec(
                    "ln -s " + SW_CALIB_TESTER_CFG_PATH + " " + SW_CALIB_CFG_PATH);
        } catch (IOException e) {
            Log.e(this.toString(), e.toString());
            alertAndQuit("Tester failed", "Tester application initialization failed");
        }
    }
}
