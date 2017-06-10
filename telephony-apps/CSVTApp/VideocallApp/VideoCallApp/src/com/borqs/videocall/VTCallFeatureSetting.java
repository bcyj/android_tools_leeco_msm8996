/*
 *Copyright (c) 2015 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.borqs.videocall;


import com.borqs.videocall.TimeConsumingPreferenceListener;
import com.borqs.videocall.VideoCallWaitingCheckBoxPreference;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import com.borqs.videocall.TimeConsumingPreferenceListener;;

public class VTCallFeatureSetting extends PreferenceActivity  {
	 private static final String BUTTON_VCW_KEY    = "button_vcw_key";
	 private VideoCallWaitingCheckBoxPreference mVCWButton;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.videocall_setting);
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

}
