/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.borqs.videocall;

import android.app.ActionBar;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.MenuItem;

import java.util.ArrayList;

//import static com.android.internal.telephony.MSimConstants.SUBSCRIPTION_KEY;

public class VTGsmUmtsAdditionalCallOptions extends
        TimeConsumingPreferenceActivity {
    private static final String LOG_TAG = "VTGsmUmtsAdditionalCallOptions";
    private final boolean DBG = true; //(PhoneApp.DBG_LEVEL >= 2);


    private static final String BUTTON_VCW_KEY    = "button_vcw_key";


    private VideoCallWaitingCheckBoxPreference mVCWButton;

    private final ArrayList<Preference> mPreferences = new ArrayList<Preference>();
    private int mInitIndex= 0;
    private int mSubscription = 0;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.gsm_umts_additional_options);
//        mSubscription = getIntent().getIntExtra(SUBSCRIPTION_KEY,
//                PhoneApp.getInstance().getDefaultSubscription());
        if (DBG)
            Log.d(LOG_TAG, "GsmUmtsAdditionalCallOptions onCreate, subscription: " + mSubscription);
        PreferenceScreen prefSet = getPreferenceScreen();




	if(isVTSupported()) {
	     Log.d(LOG_TAG,"VTSupported");
	     mVCWButton = (VideoCallWaitingCheckBoxPreference) prefSet.findPreference(BUTTON_VCW_KEY);
             mPreferences.add(mVCWButton);
	}




	    if(null != mVCWButton) {
		Log.d(LOG_TAG,"null != mVCWButton");
                mVCWButton.init(this, false);
	    }



        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);


    }

    @Override
    public void onFinished(Preference preference, boolean reading) {
        if (mInitIndex < mPreferences.size()-1 && !isFinishing()) {
            mInitIndex++;
            Preference pref = mPreferences.get(mInitIndex);

            if (pref instanceof VideoCallWaitingCheckBoxPreference) {
                ((VideoCallWaitingCheckBoxPreference) pref).init(this, false);
            }
        }
        super.onFinished(preference, reading);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {  // See ActionBar#setDisplayHomeAsUpEnabled()
		Intent intent = new Intent();
            intent.setClassName("com.borqs.videocall", "com.borqs.videocall.VTCallFeatureSetting");
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public boolean isVTSupported(){
	boolean isVTSupported = true;
	if(0 != mSubscription) {
	     Log.d(LOG_TAG, "Not in 3g supported slot");
	     return false;
	}

	try {
	     getPackageManager().getPackageInfo("com.borqs.videocall",0);
	} catch (Exception e) {
	     Log.d(LOG_TAG,"VideoCall Package NameNotFound"+e.toString());
	     isVTSupported = false;
	}
        return isVTSupported;
    }
}
