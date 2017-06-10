/*=========================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

package com.quicinc.wipoweragent;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.quicinc.wbc.WbcManager;
import com.quicinc.wbc.WbcTypes;

public class WipowerAgentActivity extends Activity implements WbcManager.WbcEventListener {
    private static final boolean DBG = true;
    private static final String TAG = "WiPwrAg-Activity";
    private static final String DIALOG_TAG = "dialog";
    private WbcManager mWbcManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (SystemProperties.getBoolean("ro.bluetooth.wipower", false) == false) {
            Log.i(TAG, "WiPower not supported");
            finish();
            return;
        }
        else {
            setContentView(R.layout.activity_wi_power_agent);
            mWbcManager = WbcManager.getInstance();
        }

        if (mWbcManager != null) {
            if (DBG) Log.v(TAG, "getWipowerCapable:" + mWbcManager.getWipowerCapable());
            if (DBG) Log.v(TAG, "getPtuPresence" + mWbcManager.getPtuPresence());
            if (DBG) Log.v(TAG, "getWipowerCharging:" + mWbcManager.getWipowerCharging());
            if (DBG) Log.v(TAG, "getChargingReqd:" + mWbcManager.getChargingRequired());
        }

        Intent intent = this.getIntent();
        if (intent.getAction().equals(WbcTypes.ACTION_SHOW_BLUETOOTH_NEEDED_UI_DIALOG) &&
                getFragmentManager().findFragmentByTag(DIALOG_TAG) == null) {
            CharSequence title = getResources().getText(R.string.bt_title);
            CharSequence message = getResources().getText(R.string.bt_message);
            BluetoothDialogFragment dlgFrag =
                    BluetoothDialogFragment.newInstance(title.toString(), message.toString());
            dlgFrag.show(getFragmentManager(), DIALOG_TAG);
        } else {
            TextView tv = (TextView) findViewById(R.id.text_view);
            tv.setVisibility(View.VISIBLE);
        }
    }

    @Override
    protected void onPause() {
        if (DBG) Log.i(TAG, "onPause");
        super.onPause();

        if (mWbcManager != null) {
            mWbcManager.unregister(this);
        }
    }

    @Override
    protected void onRestart() {
        if (DBG) Log.i(TAG, "onRestart");
        super.onRestart();

        Intent intent = this.getIntent();
        if (intent.getAction().equals(WbcTypes.ACTION_SHOW_BLUETOOTH_NEEDED_UI_DIALOG) &&
                getFragmentManager().findFragmentByTag(DIALOG_TAG) == null) {
            CharSequence title = getResources().getText(R.string.bt_title);
            CharSequence message = getResources().getText(R.string.bt_message);
            BluetoothDialogFragment dlgFrag =
                    BluetoothDialogFragment.newInstance(title.toString(), message.toString());
            dlgFrag.show(getFragmentManager(), DIALOG_TAG);
        }
    }

    @Override
    protected void onResume() {
        if (DBG) Log.i(TAG, "onResume");
        super.onResume();

        if (mWbcManager != null) {
            mWbcManager.register(this);
        }
    }

    @Override
    protected void onStop() {
        if (DBG) Log.i(TAG, "onStop");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        if (DBG) Log.i(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.wi_power_agent, menu);
        return true;
    }

    @Override
    public void onWbcEventUpdate(int what, int arg1, int arg2) {
        if (DBG) Log.d(TAG, "onWbcEventUpdate rcvd: " + what + ", " + arg1 + ", " + arg2);
    }

    private void onPositiveButtonSelected() {
        Intent intent = new Intent();
        intent.setAction(android.provider.Settings.ACTION_BLUETOOTH_SETTINGS);
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e){
            Log.w(TAG, e.getLocalizedMessage());
        }
        this.finish();
    }

    private void onNegativeButtonSelected() {
        this.finish();
    }

    public static class BluetoothDialogFragment extends DialogFragment {
        private static final String KEY_TITLE = "title";
        private static final String KEY_MESSAGE = "message";

        public interface BluetoothDialogListener {
            public void onPositiveButtonSelected();
            public void onNegativeButtonSelected();
        }

        public static BluetoothDialogFragment newInstance(String title, String message) {
            BluetoothDialogFragment dlgFrg = new BluetoothDialogFragment();
            Bundle bundle = new Bundle();
            bundle.putString(KEY_TITLE, title);
            bundle.putString(KEY_MESSAGE, message);
            dlgFrg.setArguments(bundle);
            return dlgFrg;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            if (DBG) Log.d(TAG, "onCreateDialog");
            String title = getArguments().getString(KEY_TITLE);
            String message = getArguments().getString(KEY_MESSAGE);

            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

            builder.setTitle(title).setMessage(message);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    ((WipowerAgentActivity) getActivity()).onPositiveButtonSelected();
                }
            });
            builder.setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    ((WipowerAgentActivity) getActivity()).onNegativeButtonSelected();
                }
            });

            return builder.create();
        }
    }
}
