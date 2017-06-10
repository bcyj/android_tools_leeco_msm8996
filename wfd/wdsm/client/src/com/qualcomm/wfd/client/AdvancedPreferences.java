/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

package com.qualcomm.wfd.client;

import com.qualcomm.wfd.client.R;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.Preference.OnPreferenceClickListener;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;

public class AdvancedPreferences extends PreferenceActivity {
	//implements OnSharedPreferenceChangeListener, OnPreferenceClickListener, OnClickListener {
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.advanced_preferences);

        /*SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
    	//sharedPrefs.getString(key, defValue)
        sharedPrefs.registerOnSharedPreferenceChangeListener(this);

    	Preference pref = (Preference) findPreference("search");
    	pref = (Preference) findPreference("wfd_connection_parameters");
        if (pref != null) {
        	pref.setOnPreferenceClickListener(this);
            //pref.setShouldDisableView(true);
            //pref.setEnabled(false);
        }*/
	}

	/*@Override
	public boolean onPreferenceClick(Preference preference) {

    	if (preference.getKey().equals("wfd_connection_parameters")) {
    		// this needs to contain the right info, but for now we display dummy values.
    		String _str = "Connection Type: Source\nConnected Station: station 1\nBit Rate: 12 mbps\nResolution: WVGA\n";
    		Toast.makeText(getApplicationContext(), _str, 3000).show();
    		popUpMessage(_str);
    	}
    	//else
    	//	Toast.makeText(getApplicationContext(), preference.getKey(), 3000).show();
    	return true;
    }

	private void popUpMessage(String str) {
		Toast.makeText(getApplicationContext(), "it's being called", 3000).show();
    	AlertDialog builder = new AlertDialog.Builder(getApplicationContext()).create();
		builder.setMessage(str);
		builder.setButton((CharSequence) "OK", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				//action on dialog close
			}
		});
		builder.show();
    }

	public void onSharedPreferenceChanged(SharedPreferences sharedPrefs, String key) {
    	SharedPreferences sharedPreferences = getSharedPreferences("wfd_preferences", MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        Preference pref;

    	pref = (Preference) findPreference("@string/wfd_bitrate_settings");

    	// this is for search setting
    	if (pref != null) {
    		pref.setSummary((CharSequence) "Bitrate is...");
    		pref.setShouldDisableView(true);
    		pref.setEnabled(false);
    	}
    }

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub

	}*/
}
