/**
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.agpstestmode;

import java.util.HashMap;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.LocationManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioGroup;
import static com.android.internal.telephony.TelephonyIntents.SECRET_CODE_ACTION;

public class AGPSTestMode extends Activity implements OnClickListener {
    private static final String TAG = "AGPSTestMode";
    private CheckBox mAGPSEnable;
    private CheckBox mNetEnable;
    private RadioGroup mAGPSMode;
    private RadioGroup mAGPSPosMode;
    private Button mSLPTemplate;
    private EditText mSLPAddr;
    private EditText mSLPPort;
    private CheckBox mSecureMode;
    private ContentResolver mContentResolver;
    private HashMap<String, Integer> mModeId = new HashMap<String, Integer>();
    private HashMap<String, Integer> mStartModeId = new HashMap<String, Integer>();
    private HashMap<Integer, String> mModeValue = new HashMap<Integer, String>();
    private HashMap<Integer, String> mStartModeValue = new HashMap<Integer, String>();
    private static final String AGPS_ENABLE = "agps_enable";
    private static final String AGPS_NETWORK = "agps_network";
    private static final String AGPS_MODE = "providerid";
    private static final String AGPS_POS_MODE = "resettype";
    private static final String SLP_ADDR = "host";
    private static final String SLP_PORT = "port";
    private static final String SECURE_MODE = "secure_mode";

    private static final String AGPS_LOCATION_MODE_STANDALONE = "STANDALONE";
    private static final String AGPS_LOCATION_MODE_MSB = "MSB";
    private static final String AGPS_LOCATION_MODE_MSA = "MSA";
    private static final String AGPS_START_MODE_HOT = "2";
    private static final String AGPS_START_MODE_WARM = "1";
    private static final String AGPS_START_MODE_COLD = "0";

    // CMCC assisted gps SUPL(Secure User Plane Location) server address
    private static final String ASSISTED_GPS_SUPL_HOST = "assisted_gps_supl_host";
    // CMCC agps SUPL port address
    private static final String ASSISTED_GPS_SUPL_PORT = "assisted_gps_supl_port";
    // location agps position mode,MSB or MSA
    private static final String ASSISTED_GPS_POSITION_MODE = "assisted_gps_position_mode";
    // location agps start mode,cold start or hot start.
    private static final String ASSISTED_GPS_RESET_TYPE = "assisted_gps_reset_type";
    // location agps start network,home or all
    private static final String ASSISTED_GPS_NETWORK = "assisted_gps_network";

    private static final String DEF_SLP_ADDR = "221.176.0.55";
    private static final String DEF_SLP_PORT = "7275";
    private static final String DEF_AGPS_MODE = "MSB";
    private static final String DEF_AGPS_POS_MODE = "2";
    private static final String[] SLP_TEMPLATE = {
            "221.176.0.11:777", "221.176.0.12:555"
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContentResolver = getContentResolver();
        setContentView(R.layout.agps_test_mode);
        mAGPSEnable = (CheckBox) findViewById(R.id.agps_enable);
        mNetEnable = (CheckBox) findViewById(R.id.agps_network);
        mAGPSMode = (RadioGroup) findViewById(R.id.agps_mode);
        mAGPSPosMode = (RadioGroup) findViewById(R.id.agps_position_mode);
        mSLPTemplate = (Button) findViewById(R.id.slp_template);
        //mSLPTemplate.setOnClickListener(this);
        mSLPAddr = (EditText) findViewById(R.id.slp_address);
        mSLPPort = (EditText) findViewById(R.id.slp_port);
        mSecureMode = (CheckBox) findViewById(R.id.secure_mode);
        mModeId.put(AGPS_LOCATION_MODE_STANDALONE, R.id.mode_standalone);
        mModeId.put(AGPS_LOCATION_MODE_MSB, R.id.mode_msb);
        mModeId.put(AGPS_LOCATION_MODE_MSA, R.id.mode_msa);
        mStartModeId.put(AGPS_START_MODE_HOT, R.id.position_mode_hotstart);
        mStartModeId.put(AGPS_START_MODE_WARM, R.id.position_mode_warmstart);
        mStartModeId.put(AGPS_START_MODE_COLD, R.id.position_mode_coldstart);

        mModeValue.put(R.id.mode_standalone, AGPS_LOCATION_MODE_STANDALONE);
        mModeValue.put(R.id.mode_msb, AGPS_LOCATION_MODE_MSB);
        mModeValue.put(R.id.mode_msa, AGPS_LOCATION_MODE_MSA);
        mStartModeValue.put(R.id.position_mode_hotstart, AGPS_START_MODE_HOT);
        mStartModeValue.put(R.id.position_mode_warmstart, AGPS_START_MODE_WARM);
        mStartModeValue.put(R.id.position_mode_coldstart, AGPS_START_MODE_COLD);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        MenuInflater inflate = this.getMenuInflater();
        inflate.inflate(R.menu.reset, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.reset:
                reset();
                saveAgpsParams();
                break;
            case R.id.save:
                saveAgpsParams();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        super.onResume();
        restoreConfig();
    }

    @Override
    protected void onPause() {
        super.onPause();
        saveAgpsParams();
    }

    private void reset() {
        mAGPSEnable.setChecked(true);
        mNetEnable.setChecked(true);
        mAGPSMode.check(R.id.mode_msb);
        mAGPSPosMode.check(R.id.position_mode_hotstart);
        mSLPAddr.setText(DEF_SLP_ADDR);
        mSLPPort.setText(DEF_SLP_PORT);
        mSecureMode.setChecked(true);
    }

    private void restoreConfig() {
        mAGPSEnable.setChecked(getAGPSEnable());
        mNetEnable.setChecked(true);
        mAGPSMode.check(mModeId.get(getPosMode()));
        mAGPSPosMode.check(mStartModeId.get(getResetType()));
        mSLPAddr.setText(getHost());
        mSLPPort.setText(getPort());
        mSecureMode.setChecked(true);
    }

    private boolean getAGPSEnable() {
        int enable = Settings.Global.getInt(mContentResolver,
                Settings.Global.ASSISTED_GPS_ENABLED, 0);
        return enable == 1 ? true : false;
    }

    private String getHost() {
        String supl_host = Settings.Global.getString(mContentResolver,
                ASSISTED_GPS_SUPL_HOST);
        return (null != supl_host) ? supl_host : DEF_SLP_ADDR;
    }

    private String getPort() {
        String supl_port = Settings.Global.getString(mContentResolver,
                ASSISTED_GPS_SUPL_PORT);
        return (null != supl_port) ? supl_port : DEF_SLP_PORT;
    }

    private String getResetType() {
        String agps_reset_type = Settings.Global.getString(mContentResolver,
                ASSISTED_GPS_RESET_TYPE);
        Log.e(TAG, "agps_reset_type=" + agps_reset_type);
        if (agps_reset_type != null
                && !agps_reset_type.equalsIgnoreCase("0")
                && !agps_reset_type.equalsIgnoreCase("1")
                && !agps_reset_type.equalsIgnoreCase("2")) {
            return DEF_AGPS_POS_MODE;
        }
        return (null != agps_reset_type) ? agps_reset_type : DEF_AGPS_POS_MODE;
    }

    private String getPosMode() {
        String agps_type = Settings.Global.getString(mContentResolver,
                ASSISTED_GPS_POSITION_MODE);
        return (null != agps_type) ? agps_type : DEF_AGPS_MODE;
    }

    private void SetValue(Bundle bundle)
    {
        String supl_host = bundle.getString(SLP_ADDR);
        String supl_port = bundle.getString(SLP_PORT);
        String agps_provid = bundle.getString(AGPS_MODE);
        String agps_reset_type = bundle.getString(AGPS_POS_MODE);
        if (null != supl_host && supl_host.length() > 0) {
            Settings.Global.putString(mContentResolver, ASSISTED_GPS_SUPL_HOST,
                    supl_host);
        }
        if (null != supl_port) {
            Settings.Global.putString(mContentResolver, ASSISTED_GPS_SUPL_PORT,
                    supl_port);
        }
        if (null != agps_provid && agps_provid.length() > 0) {
            Settings.Global.putString(mContentResolver, ASSISTED_GPS_POSITION_MODE,
                    agps_provid);
        }
        if (null != agps_reset_type && agps_reset_type.length() > 0) {
            Settings.Global.putString(mContentResolver, ASSISTED_GPS_RESET_TYPE,
                    agps_reset_type);
        }
        Settings.Global.putInt(mContentResolver, Settings.Global.ASSISTED_GPS_ENABLED,
                mAGPSEnable.isChecked() ? 1 : 0);
    }

    private void showSLPTemplate() {
        DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                String template = SLP_TEMPLATE[which];
                String[] addrport = template.split(":");
                mSLPAddr.setText(addrport[0]);
                mSLPPort.setText(addrport[1]);
                dialog.dismiss();
            }
        };
        int checkedItem = -1;
        String temp = mSLPAddr.getText() + ":" + mSLPPort.getText();
        for (int i = 0; i < SLP_TEMPLATE.length; i++) {
            if (temp.equalsIgnoreCase(SLP_TEMPLATE[i])) {
                checkedItem = i;
                break;
            }
        }
        AlertDialog dialog = new AlertDialog.Builder(this).setSingleChoiceItems(SLP_TEMPLATE,
                checkedItem,
                listener).setTitle(R.string.slp_templatechoose).show();
    }

    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.slp_template:
                showSLPTemplate();
                break;
        }
    }

    private void saveAgpsParams() {
        Bundle bundle = new Bundle();
        bundle.putString(SLP_ADDR, mSLPAddr.getText().toString());
        bundle.putString(SLP_PORT, mSLPPort.getText().toString());
        bundle.putString(AGPS_MODE, mModeValue.get(mAGPSMode.getCheckedRadioButtonId()));
        bundle.putString(AGPS_POS_MODE, mStartModeValue.get(mAGPSPosMode.getCheckedRadioButtonId()));
        SetValue(bundle);
        LocationManager locationmanager = (LocationManager)
                getSystemService(Context.LOCATION_SERVICE);
        boolean bRet = locationmanager.sendExtraCommand(LocationManager.GPS_PROVIDER,
                "agps_parms_changed", bundle);
        Log.d(TAG, "sendExtraCommand ret=" + bRet);
    }

    public static class StartAGPSSetting extends BroadcastReceiver {
        private static final String HOST_CODE_ENABLE = "2477738";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String host = intent.getData() != null ? intent.getData().getHost() : null;

            if (SECRET_CODE_ACTION.equals(action)) {
                if (HOST_CODE_ENABLE.equals(host)) {
                    context.startActivity(new Intent("android.intent.action.StartAGPSSetting")
                            .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
                }
            }
        }
    }

}
