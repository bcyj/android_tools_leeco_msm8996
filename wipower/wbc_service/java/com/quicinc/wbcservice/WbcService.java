/*=========================================================================
  WbcService.java
  DESCRIPTION
  Wipower Battery Control Service

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wbcservice;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.util.Log;
import android.util.Slog;
import android.os.RemoteException;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Message;
import android.provider.Settings;
import android.os.SystemProperties;
import android.os.UserHandle;

import com.quicinc.wbc.IWbcEventListener;
import com.quicinc.wbc.IWbcService;
import com.quicinc.wbc.WbcTypes;

import java.util.HashMap;
import java.util.Map;

public class WbcService extends IWbcService.Stub implements WipowerBatteryControl.WbcEventListener {
    private static boolean sDbg = false;
    private static final boolean DEBUG = true;
    private static final String TAG = "Wbc-Svc";

    private static final int MSG_INTERNAL_BASE = 1000;
    private static final int MSG_INTERNAL_START = 1 + MSG_INTERNAL_BASE;

    private static final int MSG_SYSTEM_BASE = 100;
    private static final int MSG_BATTERY_CHANGED         = 1 + MSG_SYSTEM_BASE;
    private static final int MSG_BLUETOOTH_STATE_CHANGED = 2 + MSG_SYSTEM_BASE;
    private static final int MSG_POWER_CONNECTED         = 3 + MSG_SYSTEM_BASE;
    private static final int MSG_POWER_DISCONNECTED      = 4 + MSG_SYSTEM_BASE;

    private static final int MSG_WBC_HAL_EVENT_BASE = 0;
    private static final int MSG_WBC_HAL_EVENT_WIPOWER_CAPABILITY      = 1 + MSG_WBC_HAL_EVENT_BASE;
    private static final int MSG_WBC_HAL_EVENT_PTU_PRESENCE            = 2 + MSG_WBC_HAL_EVENT_BASE;
    private static final int MSG_WBC_HAL_EVENT_WIPOWER_CHARGING_STATUS = 3 + MSG_WBC_HAL_EVENT_BASE;
    private static final int MSG_WBC_HAL_EVENT_BATTERY_CHARGING_STATUS = 4 + MSG_WBC_HAL_EVENT_BASE;

    private final Context mContext;
    private HandlerThread mHandlerThread;
    private ServiceHandler mServiceHandler;
    public WipowerBatteryControl mWbcInterface;

    private final Map<IBinder, ListenerMonitor> mClientListeners = new HashMap<IBinder, ListenerMonitor>();

    private StatusChangeReceiver mReceiver = new StatusChangeReceiver();

    public WbcService(Context context) {
        mContext = context;
        mWbcInterface = new WipowerBatteryControl(this);

        String debug = SystemProperties.get("persist.wbc.log_level", "0");
        sDbg = debug.equals("0") ? false : true;

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        if (DEBUG) {
            intentFilter.addAction("com.quicinc.wbcservice.action.WIPOWER_CAPABLE_STATUS");
            intentFilter.addAction("com.quicinc.wbcservice.action.PTU_PRESENCE_STATUS");
            intentFilter.addAction("com.quicinc.wbcservice.action.WIPOWER_CHARGING_ACTIVE_STATUS");
            intentFilter.addAction("com.quicinc.wbcservice.action.CHARGING_REQUIRED_STATUS");
        }
        context.registerReceiver(mReceiver, intentFilter);

        mHandlerThread= new HandlerThread("WbcSvcThread", Process.THREAD_PRIORITY_BACKGROUND);
        mHandlerThread.start();
        mServiceHandler = new ServiceHandler(mHandlerThread.getLooper());

        Message msg = mServiceHandler.obtainMessage();
        msg.what = MSG_INTERNAL_START;
        msg.arg1 = 0;
        msg.arg2 = 0;
        mServiceHandler.sendMessageDelayed(msg, 2000);
    }

    public void echo(int val) {
        mWbcInterface.echo(val);
    }

    public int getWipowerCapable() {
        return mWbcInterface.getWipowerCapable();
    }

    public int getPtuPresence() {
        return mWbcInterface.getPtuPresence();
    }

    public int getWipowerCharging() {
        return mWbcInterface.getWipowerCharging();
    }

    public int getChargingRequired() {
        return mWbcInterface.getChargingRequired();
    }

    private final class ListenerMonitor implements IBinder.DeathRecipient {
        private final IWbcEventListener mListener;

        public ListenerMonitor(IWbcEventListener listener) {
            mListener = listener;
        }

        public IWbcEventListener getListener() {
            return mListener;
        }

        @Override
        public void binderDied() {
            WbcService.this.unregister(this.mListener);
        }
    }

    public void register(IWbcEventListener listener) throws RemoteException {
        if (listener != null) {
            IBinder binder = listener.asBinder();
            synchronized (mClientListeners) {
                if (!mClientListeners.containsKey(binder)) {
                    ListenerMonitor listenerMonitor = new ListenerMonitor(listener);
                    binder.linkToDeath(listenerMonitor, 0);
                    mClientListeners.put(binder, listenerMonitor);
                }
            }
        }
    }

    public void unregister(IWbcEventListener listener) {
        if (listener != null) {
            IBinder binder = listener.asBinder();
            synchronized (mClientListeners) {
                ListenerMonitor listenerMonitor = mClientListeners.remove(binder);
                if (listenerMonitor != null) {
                    binder.unlinkToDeath(listenerMonitor, 0);
                }
            }
        }
    }

    @Override
    public void onWbcEventUpdate(int what, int arg1, int arg2) {
        if (sDbg) Log.v(TAG, "onWbcEventUpdate rcvd: " + what + ", " + arg1 + ", " + arg2);

        Message msg = mServiceHandler.obtainMessage();

        switch (what) {
        case WipowerBatteryControl.WBC_EVENT_TYPE_WIPOWER_CAPABLE_STATUS:
            msg.what = MSG_WBC_HAL_EVENT_WIPOWER_CAPABILITY;
            break;
        case WipowerBatteryControl.WBC_EVENT_TYPE_PTU_PRESENCE_STATUS:
            msg.what = MSG_WBC_HAL_EVENT_PTU_PRESENCE;
            break;
        case WipowerBatteryControl.WBC_EVENT_TYPE_WIPOWER_CHARGING_ACTIVE_STATUS:
            msg.what = MSG_WBC_HAL_EVENT_WIPOWER_CHARGING_STATUS;
            break;
        case WipowerBatteryControl.WBC_EVENT_TYPE_CHARGING_REQUIRED_STATUS:
            msg.what = MSG_WBC_HAL_EVENT_BATTERY_CHARGING_STATUS;
            break;
        default:
            if (sDbg) Log.w(TAG, "Rcvd unknown WBC event: " + what);
            msg.what = what;
            break;
        }

        msg.arg1 = arg1;
        msg.arg2 = arg2;
        mServiceHandler.sendMessage(msg);
    }

    private final class StatusChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = mServiceHandler.obtainMessage();

            if (intent.getAction().equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                if (sDbg) Log.v(TAG, "Bluetooth changed, extra_state: "
                        + intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)
                        + ", extra_prev_state: " + intent.getIntExtra(BluetoothAdapter.EXTRA_PREVIOUS_STATE, -1));
                msg.what = MSG_BLUETOOTH_STATE_CHANGED;
            }

            msg.arg1 = 0;
            msg.arg2 = 0;
            msg.obj = intent;

            if (DEBUG) {
                if (intent.getAction().equals("com.quicinc.wbcservice.action.PTU_PRESENCE_STATUS")) {
                    msg.what = MSG_WBC_HAL_EVENT_PTU_PRESENCE;
                    msg.arg1 = intent.getIntExtra("arg1", 0);
                    Log.v(TAG, "DBG Intent: PTU_PRESENCE_STATUS");
                } else if (intent.getAction().equals("com.quicinc.wbcservice.action.WIPOWER_CHARGING_ACTIVE_STATUS")) {
                    msg.what = MSG_WBC_HAL_EVENT_WIPOWER_CHARGING_STATUS;
                    msg.arg1 = intent.getIntExtra("arg1", 0);
                    Log.v(TAG, "DBG Intent: WIPOWER_CHARGING_ACTIVE_STATUS");
                }  else if (intent.getAction().equals("com.quicinc.wbcservice.action.CHARGING_REQUIRED_STATUS")) {
                    msg.what = MSG_WBC_HAL_EVENT_BATTERY_CHARGING_STATUS;
                    msg.arg1 = intent.getIntExtra("arg1", 0);
                    Log.v(TAG, "DBG Intent: CHARGING_REQUIRED_STATUS");
                }
            }

            mServiceHandler.sendMessage(msg);
        }
    }

    private final class ServiceHandler extends Handler {

        abstract class State {
            void onEntry() { }
            void onExit() { }
            void onBluetoothOn() { }
            void onBluetoothOff() { }
            void onWiPowerPtuPresent() { }
            void onWiPowerPtuRemoved() { }
            void onWiPowerChargingActive() { }
            void onWiPowerNotCharging() { }
        }

        class WiPowerRemoved extends State {
            void onWiPowerPtuPresent() {
                if (isBluetoothOn()) {
                    setState(mWiPowerPresentNotCharging);
                } else {
                    setState(mWiPowerPresentBluetoothOff);
                }
            }
        }

        class WiPowerPresentNotCharging extends State {
            void onEntry() {
                // check if charging is in progress and move to charging state
                if (mWbcInterface.getWipowerCharging() ==
                        WipowerBatteryControl.WBC_WIPOWER_STATUS_CHARGING_ACTIVE) {
                    setState(mWiPowerPresentCharging);
                }
            }

            void onBluetoothOff() {
                setState(mWiPowerPresentBluetoothOff);
            }

            void onWiPowerPtuRemoved() {
                setState(mWiPowerRemoved);
            }

            void onWiPowerChargingActive() {
                setState(mWiPowerPresentCharging);
            }
        }

        class WiPowerPresentCharging extends State {
            void onEntry() {
                mContext.sendBroadcastAsUser(new Intent(WbcTypes.ACTION_WIPOWER_ICON_ENABLE), UserHandle.ALL);
            }

            void onExit() {
                mContext.sendBroadcastAsUser(new Intent(WbcTypes.ACTION_WIPOWER_ICON_DISABLE), UserHandle.ALL);
            }

            void onBluetoothOff() {
                setState(mWiPowerPresentBluetoothOff);
            }

            void onWiPowerPtuRemoved() {
                setState(mWiPowerRemoved);
            }

            void onWiPowerNotCharging() {
                setState(mWiPowerPresentNotCharging);
            }
        }

        class WiPowerPresentBluetoothOff extends State {
            void onEntry() {
                mContext.sendBroadcastAsUser(new Intent(WbcTypes.ACTION_SHOW_BLUETOOTH_NEEDED_UI_DIALOG), UserHandle.ALL);
            }

            void onBluetoothOn() {
                setState(mWiPowerPresentNotCharging);
            }

            void onWiPowerPtuRemoved() {
                setState(mWiPowerRemoved);
            }
        }

        State mWiPowerRemoved;
        State mWiPowerPresentNotCharging;
        State mWiPowerPresentCharging;
        State mWiPowerPresentBluetoothOff;

        // current state
        State mState;

        public ServiceHandler(Looper looper) {
            super(looper);

            mWiPowerRemoved = new WiPowerRemoved();
            mWiPowerPresentNotCharging = new WiPowerPresentNotCharging();
            mWiPowerPresentCharging = new WiPowerPresentCharging();
            mWiPowerPresentBluetoothOff = new WiPowerPresentBluetoothOff();

            mState = mWiPowerRemoved;
        }

        void setState(State state) {
            mState.onExit();

            if (sDbg) Log.d(TAG, "State change: " + mState.getClass().getSimpleName() + " --> " + state.getClass().getSimpleName());
            mState = state;

            mState.onEntry();
        }

        boolean isBluetoothOn() {
            return Settings.Global.getInt(mContext.getContentResolver(), Settings.Global.BLUETOOTH_ON, 0) == 1;
        }

        void handleHalPtuEvent(Message msg) {
            switch (msg.arg1) {
            case WipowerBatteryControl.WBC_PTU_STATUS_NOT_PRESENT:
                mState.onWiPowerPtuRemoved();
                break;
            case WipowerBatteryControl.WBC_PTU_STATUS_PRESENT:
                mState.onWiPowerPtuPresent();
                break;
            default:
                if (sDbg) Log.w(TAG, "Rcvd unknown HAL PTU event: " + msg.arg1);
                break;
            }
        }

        void handleHalWiPowerEvent(Message msg) {
            switch (msg.arg1) {
            case WipowerBatteryControl.WBC_WIPOWER_STATUS_NOT_CHARGING:
                mState.onWiPowerNotCharging();
                break;
            case WipowerBatteryControl.WBC_WIPOWER_STATUS_CHARGING_ACTIVE:
                mState.onWiPowerChargingActive();
                break;
            default:
                if (sDbg) Log.w(TAG, "Rcvd unknown HAL WiPower event: " + msg.arg1);
                break;
            }
        }

        void handleHalBatteryEvent(Message msg) {
            switch (msg.arg1) {
            case WipowerBatteryControl.WBC_BATTERY_STATUS_CHARGING_NOT_REQUIRED:
                break;
            case WipowerBatteryControl.WBC_BATTERY_STATUS_CHARGING_REQUIRED:
                break;
            default:
                if (sDbg) Log.w(TAG, "Rcvd unknown HAL Battery event: " + msg.arg1);
                break;
            }
        }

        void handleBluetoothEvent(Message msg) {
            if (msg == null || msg.obj == null) {
                Log.e(TAG, "BT event invalid!!");
                return;
            }

            Intent intent = (Intent) msg.obj;
            int btState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1);

            switch (btState) {
                case BluetoothAdapter.STATE_ON:
                    mState.onBluetoothOn();
                    break;
                case BluetoothAdapter.STATE_OFF:
                    mState.onBluetoothOff();
                    break;
                case BluetoothAdapter.STATE_TURNING_ON:
                    break;
                case BluetoothAdapter.STATE_TURNING_OFF:
                    break;
                default:
                    break;
            }
        }

        @Override
        public void handleMessage(Message msg) {
            if (sDbg) Log.d(TAG, "Handler rcvd msg: " + msg.what);

            switch (msg.what) {
            case MSG_INTERNAL_START:
                if (sDbg) Log.d(TAG, "Msg Start");

                int ptuStatus = mWbcInterface.getPtuPresence();
                if (ptuStatus == WipowerBatteryControl.WBC_PTU_STATUS_PRESENT) {
                    if (isBluetoothOn()) {
                        setState(mWiPowerPresentNotCharging);
                    } else {
                        setState(mWiPowerPresentBluetoothOff);
                    }
                } else {
                    ; // do nothing as we default state is WiPowerRemoved
                }
                break;
            case MSG_BLUETOOTH_STATE_CHANGED:
                handleBluetoothEvent(msg);
                break;
            case MSG_WBC_HAL_EVENT_WIPOWER_CAPABILITY:
                notifyClients(msg);
                break;
            case MSG_WBC_HAL_EVENT_PTU_PRESENCE:
                handleHalPtuEvent(msg);
                notifyClients(msg);
                break;
            case MSG_WBC_HAL_EVENT_WIPOWER_CHARGING_STATUS:
                handleHalWiPowerEvent(msg);
                notifyClients(msg);
                break;
            case MSG_WBC_HAL_EVENT_BATTERY_CHARGING_STATUS:
                handleHalBatteryEvent(msg);
                notifyClients(msg);
                break;
            default:
                if (sDbg) Log.w(TAG, "Rcvd Unknown msg: " + msg.what);
                notifyClients(msg);
                break;
            }
        }

        void notifyClients(Message msg) {
            synchronized (mClientListeners) {
                for (Map.Entry<IBinder, ListenerMonitor> entry : mClientListeners.entrySet()) {
                    IWbcEventListener listener = entry.getValue().getListener();
                    try {
                        listener.onWbcEventUpdate(msg.what, msg.arg1, msg.arg2);
                    } catch (RemoteException e) {
                        Log.w(TAG, e.getLocalizedMessage());
                        WbcService.this.unregister(listener);
                    }
                }
            }
        }
    }
}