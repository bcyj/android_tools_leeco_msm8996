/* ------------------------------------------------------------------
 * Copyright (C) 2012 BORQS Software Solutions Pvt Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */


package com.borqs.videocall;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;

import android.app.ActionBar;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.util.Log;
import android.view.MenuItem;

import java.util.ArrayList;


public class VTCallForwardOptions extends TimeConsumingPreferenceActivity {
    private static final String LOG_TAG = "VTCallForwardOptions";
    private final boolean DBG = true;

    private static final String NUM_PROJECTION[] = {Phone.NUMBER};

    private static final String BUTTON_VT_CFU_KEY   = "button_vt_cfu_key";
    private static final String BUTTON_VT_CFB_KEY   = "button_vt_cfb_key";
    private static final String BUTTON_VT_CFNRY_KEY = "button_vt_cfnry_key";
    private static final String BUTTON_VT_CFNRC_KEY = "button_vt_cfnrc_key";

    private static final String KEY_TOGGLE = "toggle";
    private static final String KEY_STATUS = "status";
    private static final String KEY_NUMBER = "number";

    private CallForwardEditPreference mButtonCFU;
    private CallForwardEditPreference mButtonCFB;
    private CallForwardEditPreference mButtonCFNRy;
    private CallForwardEditPreference mButtonCFNRc;

    private final ArrayList<CallForwardEditPreference> mPreferences =
            new ArrayList<CallForwardEditPreference> ();
    private int mInitIndex= 0;

    private boolean mFirstResume;
    private Bundle mIcicle;
    private int mSubscription = 0;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.videocall_callforward_options);

        PreferenceScreen prefSet = getPreferenceScreen();
        mButtonCFU   = (CallForwardEditPreference) prefSet.findPreference(BUTTON_VT_CFU_KEY);
        mButtonCFB   = (CallForwardEditPreference) prefSet.findPreference(BUTTON_VT_CFB_KEY);
        mButtonCFNRy = (CallForwardEditPreference) prefSet.findPreference(BUTTON_VT_CFNRY_KEY);
        mButtonCFNRc = (CallForwardEditPreference) prefSet.findPreference(BUTTON_VT_CFNRC_KEY);

        mButtonCFU.setParentActivity(this, mButtonCFU.reason);
        mButtonCFB.setParentActivity(this, mButtonCFB.reason);
        mButtonCFNRy.setParentActivity(this, mButtonCFNRy.reason);
        mButtonCFNRc.setParentActivity(this, mButtonCFNRc.reason);

        mButtonCFU.setParameter(this, mSubscription);
        mButtonCFB.setParameter(this, mSubscription);
        mButtonCFNRy.setParameter(this, mSubscription);
        mButtonCFNRc.setParameter(this, mSubscription);

        mPreferences.add(mButtonCFU);
        mPreferences.add(mButtonCFB);
        mPreferences.add(mButtonCFNRy);
        mPreferences.add(mButtonCFNRc);

        // we wait to do the initialization until onResume so that the
        // TimeConsumingPreferenceActivity dialog can display as it
        // relies on onResume / onPause to maintain its foreground state.

        mFirstResume = true;
        mIcicle = icicle;

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mFirstResume) {

            if (mIcicle != null) {
                mInitIndex = mPreferences.size();

                for (CallForwardEditPreference pref : mPreferences) {
                    Bundle bundle = mIcicle.getParcelable(pref.getKey());
                    pref.setToggled(bundle.getBoolean(KEY_TOGGLE));
                    CallForwardInfo cf = new CallForwardInfo();
                    cf.number = bundle.getString(KEY_NUMBER);
                    cf.status = bundle.getInt(KEY_STATUS);
                    pref.handleCallForwardResult(cf);
                    pref.init(true);
                }
            }
            for (CallForwardEditPreference pref : mPreferences) {
                pref.setSummaryOff(R.string.sum_cf_update);
            }

            mFirstResume = false;
            mIcicle=null;
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        for (CallForwardEditPreference pref : mPreferences) {
            Bundle bundle = new Bundle();
            bundle.putBoolean(KEY_TOGGLE, pref.isToggled());
            if (pref.callForwardInfo != null) {
                bundle.putString(KEY_NUMBER, pref.callForwardInfo.number);
                bundle.putInt(KEY_STATUS, pref.callForwardInfo.status);
            }
            outState.putParcelable(pref.getKey(), bundle);
        }
    }

    @Override
    public void onFinished(Preference preference, boolean reading) {

//        if (mInitIndex < mPreferences.size()-1 && !isFinishing()) {
//            mInitIndex++;
//            mPreferences.get(mInitIndex).init(false);
//        }

        super.onFinished(preference, reading);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (DBG) Log.d(LOG_TAG, "onActivityResult: done");
        if (resultCode != RESULT_OK) {
            if (DBG) Log.d(LOG_TAG, "onActivityResult: contact picker result not OK.");
            return;
        }
        Cursor cursor = getContentResolver().query(data.getData(),
                NUM_PROJECTION, null, null, null);
        if ((cursor == null) || (!cursor.moveToFirst())) {
            if (DBG) Log.d(LOG_TAG, "onActivityResult: bad contact data, no results found.");
            return;
        }

        switch (requestCode) {
            case CommandsInterface.CF_REASON_UNCONDITIONAL:
                mButtonCFU.onPickActivityResult(cursor.getString(0));
                break;
            case CommandsInterface.CF_REASON_BUSY:
                mButtonCFB.onPickActivityResult(cursor.getString(0));
                break;
            case CommandsInterface.CF_REASON_NO_REPLY:
                mButtonCFNRy.onPickActivityResult(cursor.getString(0));
                break;
            case CommandsInterface.CF_REASON_NOT_REACHABLE:
                mButtonCFNRc.onPickActivityResult(cursor.getString(0));
                break;
            default:
                // TODO: may need exception here.
        }
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
}
