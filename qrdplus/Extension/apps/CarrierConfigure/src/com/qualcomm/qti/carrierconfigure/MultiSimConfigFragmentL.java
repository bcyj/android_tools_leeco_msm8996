/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.carrierconfigure.Actions.UpdateSimMode;
import com.qualcomm.qti.carrierconfigure.Utils.MyAlertDialog;
import com.qualcomm.qti.carrierconfigure.Utils.WaitDialog;

public class MultiSimConfigFragmentL extends RadioPreferenceFragmentL
        implements MyAlertDialog.OnAlertDialogButtonClick {
    private static final String TAG = "MultiSimConfigFragmentL";

    private static final int MSG_UPDATE = MSG_BASE + 1;

    public MultiSimConfigFragmentL() {
        super();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // As we defined the preference's key same as the property value,
        // so get the current and selected preference key from the property.
        mCurrentPreferenceKey.add(Utils.getSimMode().toLowerCase());
        mSelectedPreferenceKey.put(Utils.getSimMode().toLowerCase(),
                Utils.getSimMode().toLowerCase());

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.multisim_config_preference_l);
    }
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        RadioGroupPreferenceCategory category = new RadioGroupPreferenceCategory(getActivity());
        category.setTitle(R.string.multisim_category_text);
        category.setKey("multisim_category_key");
        mPreferenceScreen.addPreference(category);

        //Create the preference of this carrier.
        RadioButtonPreference preference = new RadioButtonPreference(getActivity());
        preference.setKey("ssss");
        preference.setTitle(R.string.sim_ssss_text);
        category.addPreference(preference);

        preference = new RadioButtonPreference(getActivity());
        preference.setKey("dsds");
        preference.setTitle(R.string.sim_dsds_text);
        category.addPreference(preference);

        preference = new RadioButtonPreference(getActivity());
        preference.setKey("dsda");
        preference.setTitle(R.string.sim_dsda_text);
        category.addPreference(preference);

    }

    @Override
    public void onResume() {
        super.onResume();

        // Update the selected status.
        ((RadioGroupPreferenceCategory)mPreferenceScreen.getPreference(0)).
                setCheckedPreference(mSelectedPreferenceKey.get(mCurrentPreferenceKey.get(0)));
    }

    @Override
    protected void handleMessage(Message msg) {
        if (msg.what == MSG_UPDATE) {
            if (Utils.DEBUG) Log.d(TAG, "Reboot now! For user press yes.");
            // Show the "Please wait ..." dialog.
            WaitDialog wait = Utils.WaitDialog.newInstance();
            wait.show(getFragmentManager(), WaitDialog.TAG_LABEL);

            // For user press OK, update the prop value.
            Intent intent = new Intent(UpdateSimMode.ACTION_SIM_MODE_UPDATE);
            intent.setClass(getActivity(), UpdateSimMode.class);
            intent.putExtra(UpdateSimMode.EXTRA_NEW_MODE, mSelectedPreferenceKey
                    .get(mCurrentPreferenceKey.get(0)));
            getActivity().startService(intent);
        }
    }

    @Override
    protected void onSelectedChanged() {
        // Show the dialog to alert the user.
        MyAlertDialog dialog = MyAlertDialog.newInstance(this,
                R.string.alert_update_sim_mode_title,
                R.string.alert_update_sim_mode_text);
        dialog.show(getFragmentManager(), MyAlertDialog.TAG_LABEL);
    }

    @Override
    public void onAlertDialogButtonClick(int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                sendEmptyMessage(MSG_UPDATE);
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                // For user press CANCEL, then reset the value and update the display.
                resetSelection();
                break;
        }
    }
}
