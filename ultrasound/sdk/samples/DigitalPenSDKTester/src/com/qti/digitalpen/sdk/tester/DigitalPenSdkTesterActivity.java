/*===========================================================================
                           DigitalPenSdkTesterActivity.java

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.digitalpen.sdk.tester;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Area;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.BatteryStateChangedListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Feature;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.MicBlockedListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OffScreenMode;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OnSideChannelDataListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.PowerStateChangedListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Settings;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelData;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelMapping;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.BatteryStateChangedListener.BatteryState;
import com.qti.snapdragon.sdk.digitalpen.PenEnabledChecker;

import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnFocusChangeListener;
import android.view.View.OnHoverListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.EditorInfo;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.TextView.OnEditorActionListener;

public class DigitalPenSdkTesterActivity extends Activity {
    private static final String TAG = "DigitalPenSdkTesterActivity";

    private static Integer numBlockedMics;

    private static PowerStateChangedListener.PowerState currentPowerState;

    protected static BatteryState batteryState;

    private EditText onScreenHoverCustomRange;

    private HandleMotionEvent motionEventHandler;

    private OffScreenPresentation offscreenPresentation;

    private DigitalPenManager mgr;

    private class SideChannelDataListener implements OnSideChannelDataListener {
        private Area area;

        public SideChannelDataListener(Area area) {
            this.area = area;
        }

        @Override
        public void onDigitalPenData(final SideChannelData data) {
            pointCount++;
            runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    setOnScreenDataPoints(area, data);
                }
            });
        }

    }

    private final OnSideChannelDataListener onScreenListener = new SideChannelDataListener(
            Area.ON_SCREEN);

    private final OnSideChannelDataListener offScreenListener = new SideChannelDataListener(
            Area.OFF_SCREEN);

    private final OnSideChannelDataListener allListener = new SideChannelDataListener(Area.ALL);

    private TextView onScreenDataText;

    private TextView listenerText;

    private TextView motionEventDataText;

    private int pointCount;

    private CompoundButton onScreenHoverCustomButton;

    private CompoundButton offScreenHoverOffButton;

    private ViewGroup offScreenHoverRadioGroup;

    private CompoundButton offScreenHoverCustomButton;

    private EditText offScreenHoverCustomRange;

    private Settings settings;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        motionEventHandler = new HandleMotionEvent();
        motionEventDataText = (TextView) findViewById(R.id.textMotionEventCount);
        motionEventHandler.setOnMotionDataEvents(null, null);

        View topView = findViewById(R.id.LinearLayout1);
        topView.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                motionEventHandler.setOnMotionDataEvents("Main display touch event: ", event);
                return true;
            }
        });
        topView.setOnHoverListener(new OnHoverListener() {

            @Override
            public boolean onHover(View v, MotionEvent event) {
                motionEventHandler.setOnMotionDataEvents("Main display hover event: ", event);
                return true;
            }
        });

        onScreenDataText = (TextView) findViewById(R.id.textSideBandEventCount);
        listenerText = (TextView) findViewById(R.id.textListeners);
        onScreenHoverCustomButton = (CompoundButton) findViewById(R.id.radioOnScreenHoverEnableCustom);
        onScreenHoverCustomRange = (EditText) findViewById(R.id.editTextOnScreenHoverEnableCustom);

        offScreenHoverCustomButton = (CompoundButton) findViewById(R.id.radioOffScreenHoverEnableCustom);
        offScreenHoverCustomRange = (EditText) findViewById(R.id.editTextOffScreenHoverEnableCustom);

        offScreenHoverOffButton = (CompoundButton) findViewById(R.id.radioOffScreenHoverOff);
        offScreenHoverRadioGroup = (ViewGroup) findViewById(R.id.radioGroupOffScreenHover);

        setEditDoneListeners(onScreenHoverCustomRange);
        setEditDoneListeners(offScreenHoverCustomRange);

        setOnScreenDataPoints(null, null);

        if (!DigitalPenManager.isFeatureSupported(Feature.BASIC_DIGITAL_PEN_SERVICES)) {
            Toast.makeText(this, "Basic pen services not supported on this device.",
                    Toast.LENGTH_LONG).show();
        }

        DigitalPenManager.logVersions();

        mgr = new DigitalPenManager(getApplication());
        mgr.setMicBlockedListener(new MicBlockedListener() {

            @Override
            public void onMicBlocked(int numBlockedMics) {
                DigitalPenSdkTesterActivity.numBlockedMics = numBlockedMics;
                updateListenerReport();
            }
        });
        mgr.setPowerStateChangedListener(new PowerStateChangedListener() {

            @Override
            public void onPowerStateChanged(PowerState currentState, PowerState lastState) {
                DigitalPenSdkTesterActivity.currentPowerState = currentState;
                // note: not doing anything with lastState
                updateListenerReport();
            }
        });
        mgr.setBatteryStateChangedListener(new BatteryStateChangedListener() {

            @Override
            public void onBatteryStateChanged(BatteryState state) {
                DigitalPenSdkTesterActivity.batteryState = state;
                updateListenerReport();
            }
        });
        settings = mgr.getSettings();
        setWidgetDefaults();
    }

    private void updateListenerReport() {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                if (listenerText == null) {
                    return;
                }
                listenerText.setText("Mics blocked: " + numBlockedMics + ", power state: "
                        + currentPowerState + ", battery state: " + batteryState);
            }
        });
    }

    private void setEditDoneListeners(EditText editText) {
        editText.setOnEditorActionListener(new OnEditorActionListener() {

            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    applySettings();
                }
                return false;
            }
        });
        editText.setOnFocusChangeListener(new OnFocusChangeListener() {

            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    // switch to custom hover if user clicks range
                    switch (v.getId()) {
                        case R.id.editTextOnScreenHoverEnableCustom:
                            onScreenHoverCustomButton.setChecked(true);
                            break;
                        case R.id.editTextOffScreenHoverEnableCustom:
                            offScreenHoverCustomButton.setChecked(true);
                            break;
                    }
                }
                applySettings();
            }
        });
    }

    private void setWidgetDefaults() {
        onScreenHoverCustomRange.setText(Integer.toString(settings.getOnScreenHoverMaxDistance()));
        int offScreenHoverMaxDistance = settings.getOffScreenHoverMaxDistance();
        if (offScreenHoverMaxDistance == -1) {
            offScreenHoverCustomRange.setText("400");
        } else {
            offScreenHoverCustomRange.setText(Integer.toString(offScreenHoverMaxDistance));

        }
        updateWidgetsForOffScreenModeChange(settings.getOffScreenMode());
        updateListenerReport();
    }

    /*
     * (non-Javadoc)
     * @see com.qti.snapdragon.sdk.digitalpen.DigitalPenActivity#onResume()
     */
    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();

        createOffscreenPresentation();

        PenEnabledChecker.checkEnabledAndLaunchSettings(this);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        dismissOffScreenPresentation();
        super.onDestroy();
    }

    private void setOnScreenDataPoints(Area area, SideChannelData data) {
        if (data == null) {
            onScreenDataText.setText("Side-Channel Points: " + pointCount);
        } else {
            String msg = "Side-Channel Points: " + pointCount + ", last point: " + area + " "
                    + data;
            if (settings.getSideChannelMapping(area) == SideChannelMapping.RAW) {
                final double Q_FACTOR = Math.pow(2.0, 30.0);
                final double xTilt = data.xTilt / Q_FACTOR;
                final double yTilt = data.yTilt / Q_FACTOR;
                final double zTilt = data.zTilt / Q_FACTOR;
                msg = msg.concat("\nxTilt: " + xTilt
                        + "\nyTilt: " + yTilt
                        + "\nzTilt: " + zTilt);
            } else {
                msg = msg.concat("\nxTilt: " + data.xTilt
                        + "\nyTilt: " + data.yTilt);
            }
            onScreenDataText.setText(msg);
        }
    }

    private void applySettings() {
        if (onScreenHoverCustomButton.isChecked()) {
            try {
                int range = Integer.parseInt(onScreenHoverCustomRange.getText()
                        .toString());
                settings.setOnScreenHoverEnabled(range);
            } catch (NumberFormatException x) {
                Toast.makeText(this, "On-screen hover range, " + onScreenHoverCustomRange.getText()
                        + " not a number", Toast.LENGTH_LONG).show();
            }
        }
        if (offScreenHoverCustomButton.isChecked()) {
            try {
                int range = Integer.parseInt(offScreenHoverCustomRange.getText()
                        .toString());
                settings.setOffScreenHoverEnabled(range);
            } catch (NumberFormatException x) {
                Toast.makeText(this,
                        "Off-screen hover range, " + offScreenHoverCustomRange.getText()
                                + " not a number", Toast.LENGTH_LONG).show();
            }
        }
        boolean success = settings.apply();
        String toastText = success ? "Settings applied"
                : "Apply Settings failed! See logcat.";
        int toastLength = success ? Toast.LENGTH_SHORT : Toast.LENGTH_LONG;

        Toast.makeText(this, toastText, toastLength).show();
    }

    public void onClickEraserBypassRadio(View v) {
        switch (v.getId()) {
            case R.id.radioEraserEnable:
                settings.setEraserBypass();
                break;
            case R.id.radioEraserOff:
                settings.setEraserBypassDisabled();
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        applySettings();
    }

    class HandleMotionEvent {
        public void setOnMotionDataEvents(String eventDestination, MotionEvent event) {
            if (eventDestination == null) {
                motionEventDataText.setText(" ");
            } else {

                motionEventDataText.setText(eventDestination + getMotionEventString(event));
            }
        }

        private String getMotionEventString(MotionEvent event) {
            StringBuilder msg = new StringBuilder();

            final int pointerCount = event.getPointerCount();

            msg.append(", buttonState=").append(event.getButtonState());
            msg.append(", pressure=").append(event.getPressure());
            msg.append(", metaState=").append(event.getMetaState());
            msg.append(", flags=0x").append(Integer.toHexString(event.getFlags()));
            msg.append(", edgeFlags=0x").append(Integer.toHexString(event.getEdgeFlags()));
            msg.append(", pointerCount=").append(pointerCount);
            msg.append(", historySize=").append(event.getHistorySize());
            msg.append("\neventTime=").append(event.getEventTime());
            msg.append(", downTime=").append(event.getDownTime());
            msg.append(", deviceId=").append(event.getDeviceId());
            msg.append(", source=0x").append(Integer.toHexString(event.getSource()));
            msg.append(", tilt=").append(event.getAxisValue(MotionEvent.AXIS_TILT)).append("\n");

            for (int i = 0; i < pointerCount; i++) {
                msg.append(", id[").append(i).append("]=").append(event.getPointerId(i));
                msg.append(", toolType[").append(i).append("]=").append(event.getToolType(i));
                msg.append("\nx[").append(i).append("]=").append(event.getX(i));
                msg.append("\ny[").append(i).append("]=").append(event.getY(i));
                msg.append("\nz[").append(i).append("]=")
                        .append(event.getAxisValue(MotionEvent.AXIS_DISTANCE));
            }
            msg.append(" }");
            return msg.toString();
        }
    };

    private void createOffscreenPresentation() {
        Display offscreenDisplay = findOffscreenDigitalPenDisplay(this);

        offscreenPresentation = new OffScreenPresentation(this, offscreenDisplay,
                motionEventHandler);
        offscreenPresentation.show();
    }

    private void dismissOffScreenPresentation() {
        if (offscreenPresentation != null) {
            offscreenPresentation.dismiss();
        }
    }

    private Display findOffscreenDigitalPenDisplay(Context ctxt) {
        DisplayManager displayMgr = (DisplayManager) ctxt.getSystemService(Context.DISPLAY_SERVICE);
        Display[] displays = displayMgr.getDisplays();
        Display offscreenDisplay = null;
        for (Display display : displays) {
            String displayName = display.getName();
            Log.d(TAG, "Display " + display.getDisplayId() + ": " + displayName);
            if (displayName.contains("Digital Pen")) {
                offscreenDisplay = display;
                break;
            }
        }

        if (offscreenDisplay == null) {
            throw new RuntimeException("No Digital Pen display found!");
        }
        return offscreenDisplay;
    }

    public void onClickOnScreenHoverRadio(View v) {
        switch (v.getId()) {
            case R.id.radioOnScreenHoverEnable:
                settings.setOnScreenHoverEnabled();
                break;
            case R.id.radioOnScreenHoverOff:
                settings.setOnScreenHoverDisabled();
                break;
            case R.id.radioOnScreenHoverEnableCustom:
                // applyConfiguration reads the custom value and changes the
                // mode
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        applySettings();
    }

    public void onClickOffScreenHoverRadio(View v) {
        switch (v.getId()) {
            case R.id.radioOffScreenHoverEnable:
                settings.setOffScreenHoverEnabled();
                break;
            case R.id.radioOffScreenHoverOff:
                settings.setOffScreenHoverDisabled();
                break;
            case R.id.radioOffScreenHoverEnableCustom:
                // applyConfiguration reads the custom value and changes the
                // mode
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        applySettings();
    }

    public void onClickOnScreenSideChannelRadio(View v) {
        SideChannelMapping mapping;
        switch (v.getId()) {
            case R.id.radioOnScreenSideChannelOff:
                mapping = SideChannelMapping.DISABLED;
                break;
            case R.id.radioOnScreenSideChannelScaled:
                mapping = SideChannelMapping.SCALED;
                break;
            case R.id.radioOnScreenSideChannelRaw:
                mapping = SideChannelMapping.RAW;
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        if (mapping == SideChannelMapping.DISABLED) {
            mgr.unregisterSideChannelEventListener(Area.ON_SCREEN);
        } else {
            mgr.registerSideChannelEventListener(Area.ON_SCREEN, onScreenListener);
        }
        settings.setSideChannelMapping(Area.ON_SCREEN, mapping);
        applySettings();
    }

    public void onClickOffScreenSideChannelRadio(View v) {
        SideChannelMapping mapping;
        switch (v.getId()) {
            case R.id.radioOffScreenSideChannelOff:
                mapping = SideChannelMapping.DISABLED;
                break;
            case R.id.radioOffScreenSideChannelScaled:
                mapping = SideChannelMapping.SCALED;
                break;
            case R.id.radioOffScreenSideChannelRaw:
                mapping = SideChannelMapping.RAW;
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        if (mapping == SideChannelMapping.DISABLED) {
            mgr.unregisterSideChannelEventListener(Area.OFF_SCREEN);
        } else {
            mgr.registerSideChannelEventListener(Area.OFF_SCREEN, offScreenListener);
        }
        settings.setSideChannelMapping(Area.OFF_SCREEN, mapping);
        applySettings();
    }

    public void onClickAllSideChannelRadio(View v) {
        SideChannelMapping mapping;
        switch (v.getId()) {
            case R.id.radioAllSideChannelOff:
                mapping = SideChannelMapping.DISABLED;
                break;
            case R.id.radioAllSideChannelRaw:
                mapping = SideChannelMapping.RAW;
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        if (mapping == SideChannelMapping.DISABLED) {
            mgr.unregisterSideChannelEventListener(Area.ALL);
        } else {
            mgr.registerSideChannelEventListener(Area.ALL, allListener);
        }
        settings.setSideChannelMapping(Area.ALL, mapping);
        applySettings();
    }

    public void onClickOffscreenRadio(View v) {
        OffScreenMode mode;
        switch (v.getId()) {
            case R.id.radioButtonOffscreenDuplicate:
                mode = OffScreenMode.DUPLICATE;
                enableOffScreenSideChannel(true);
                break;
            case R.id.radioButtonOffscreenExtend:
                mode = OffScreenMode.EXTEND;
                enableOffScreenSideChannel(true);
                break;
            case R.id.radioButtonOffscreenDisable:
                mode = OffScreenMode.DISABLED;
                offScreenHoverOffButton.setChecked(true);
                enableOffScreenSideChannel(false);
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        updateWidgetsForOffScreenModeChange(mode);
        settings.setOffScreenMode(mode);
        applySettings();
    }

    private void enableOffScreenSideChannel(boolean enabled) {
        findViewById(R.id.radioOffScreenSideChannelRaw).setEnabled(enabled);
        findViewById(R.id.radioOffScreenSideChannelScaled).setEnabled(enabled);
    }

    private void updateWidgetsForOffScreenModeChange(OffScreenMode mode) {
        if (mode == OffScreenMode.DISABLED) {
            offScreenHoverOffButton.setChecked(true);
            enableAllChildren(false, offScreenHoverRadioGroup);
        } else {
            enableAllChildren(true, offScreenHoverRadioGroup);
        }
    }

    private void enableAllChildren(boolean enable, ViewGroup vg) {
        for (int i = 0; i < vg.getChildCount(); i++) {
            View child = vg.getChildAt(i);
            child.setEnabled(enable);
            if (child instanceof ViewGroup) {
                enableAllChildren(enable, (ViewGroup) child);
            }
        }
    }

}
