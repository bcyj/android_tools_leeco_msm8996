/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.extcarrierpack;

import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.app.AlertDialog;
import android.content.DialogInterface;

//To display the preferred PLMN list from SIM
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccConstants;

import java.util.HashMap;
import java.util.List;

/*
    This contains support for displaying Preferred Networks list.
    As per 3GPP specs TS 11.11, EF PLMN sel is an elementary file in the SIM.
    This contains minimum 'n' PLMN MCC & MNC values. This information is
    determined by user/operator about the preferred PLMN of the user in
    priority order.
*/

public class PreferredNetworksActivity extends PreferenceActivity {
    private static final String LOG_TAG = "PreferredNetworks";
    private static final boolean DBG = true;

    //String keys for preference lookup
    private static final String PREFERRED_NETWORKS_KEY = "preferred_networks_key";

    private PreferenceGroup mNetworkList;
    protected boolean mIsForeground = false;

    private static final int EVENT_GET_PLMN_SEL_DONE = 100;

    private StringBuffer mData;
    private IccFileHandler mFh;
    private UiccController mUiccController = null;

    private HashMap<String, String> mMncMccValue = new HashMap<String, String>();
    private int subscription=0;
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.preferred_networks);
        mNetworkList = (PreferenceGroup) getPreferenceScreen().findPreference(PREFERRED_NETWORKS_KEY);

        mData = new StringBuffer();

        // Read from SIM
        mUiccController = UiccController.getInstance();
        //private PhoneBase mPhone;
        if (null != mUiccController) {
            UiccCardApplication uiccApplication =
                    mUiccController.getUiccCardApplication(UiccController.APP_FAM_3GPP);
            if (null != uiccApplication) {
                mFh = uiccApplication.getIccFileHandler();
                mFh.loadEFTransparent(IccConstants.EF_PLMN_SEL,
                        mHandler.obtainMessage(EVENT_GET_PLMN_SEL_DONE));
            }
        }
        populateKnownMccMncValue();
    }

    @Override
    public void onResume() {
        super.onResume();
        mIsForeground = true;
    }

    @Override
    public void onPause() {
        super.onPause();
        mIsForeground = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            byte[] data;

            switch (msg.what) {
                case EVENT_GET_PLMN_SEL_DONE:
                default:
                    log("EVENT_GET_PLMN_SEL_DONE");

                ar = (AsyncResult)msg.obj;

                if (ar.exception != null) {
                    log("ar.exception != null. display error");
                    displayError();
                    break;
                }

                data = (byte[])ar.result;
                mData.append(IccUtils.bytesToHexString(data));
                getPreferenceScreen().setEnabled(true);
                loadPreferredNetworkList();
                break;
            }
        }
    };

    private void populateKnownMccMncValue()
    {
        log("populateKnownMccMncValue");

        int index = 0;
        String[] mccmncArray = getResources().getStringArray(R.array.plmn_mcc_mnc);
        String[] operatorNameArray = getResources().getStringArray(R.array.plmn_name);
        mMncMccValue.clear();

        /*
         * If the length of the operator name array & MCC-MNC array is same,
         * proceed to display the list. Otherwise finish the activity.
         */
        if (mccmncArray.length != operatorNameArray.length) {
            log("MCC-MNC array and operator name array are incorrect. Exit");
            finish();
            return;
        }

        //Populate the hash map mMncMccValue to be used for search
        for ( index=0; index < mccmncArray.length ; index++ ) {
            log(" mccmnc = " + mccmncArray[index]
                    + "   operatorName : " + operatorNameArray[index]);
            mMncMccValue.put(mccmncArray[index], operatorNameArray[index]);
        }
    }

    private void loadPreferredNetworkList() {
        int length;
        StringBuffer mcc, mnc,m_temp ;

        length = mData.length();
        String temp = new String();
        String string_to_display,mnc_to_display,search_key;
        Preference carrier = null;

        log("MCC/MNC values in mData = " + mData);

        if (length == 0) displayError();

        for(int i = 0; i < length; i+=6) {
            /* Logic of parsing MCC & MNC value
               3GPP TS 11.14, Section 10.3.4 : EF PLMN Sel file
               -Contains MCC code followed by MNC.
               -Excess bytes will be populated with FF
               -1st PLMN is of highest priority. Length of each MCC & MNC
                 string is 3 bytes
               -Eg : If "246" is MCC and "81" is MNC, contents of SIM file
                is as follows:
                Bytes 1‑3: '42' 'F6' '18'
            */
            /* MCC value is a 3 digit number. Reading the 1st 2 digits here*/
            temp = mData.substring(i, i+2);

            /*Extra bytes are set to FF. Hence ignore them*/
            if(temp.matches("ff") || temp.matches("FF")) {
                continue;
            }

            carrier = new Preference(PreferredNetworksActivity.this, null);

            mcc = new StringBuffer(temp);
            mcc = mcc.reverse();

            //Extract next nibble from 2nd byte
            temp = mData.substring(i+2, i+4);
            m_temp = new StringBuffer(temp); //Contains 1 nibble of MCC & 1 of MNC
            m_temp = m_temp.reverse(); //1st nibble is of MCC & 2nd nibble is of MNC
            mcc = mcc.append(m_temp.charAt(0));

            //MNC value
            mnc = new StringBuffer(m_temp.charAt(1));
            temp = mData.substring(i+4, i+6);
            m_temp = new StringBuffer(temp);
            m_temp = m_temp.reverse();
            mnc = mnc.append(m_temp);

            /*Formatting of string for Display & to search for preferred
              network name in Hash map*/
            if ('f' == mnc.charAt(0)) {
                mnc = mnc.deleteCharAt(0);
            }

            if ('f' == mnc.charAt(1)) {
                mnc = mnc.deleteCharAt(1);
            }

            string_to_display = new String(mcc);
            mnc_to_display = new String(mnc);
            search_key = string_to_display.concat(mnc_to_display);

            String operator_name = mMncMccValue.get(search_key);
            if (operator_name == null) {
                /* Display the mcc & mnc value */
                string_to_display = string_to_display + " , " + mnc_to_display;
                carrier.setTitle(string_to_display);
            } else {
                carrier.setTitle(operator_name);
            }

            carrier.setPersistent(false);
            mNetworkList.addPreference(carrier);
         }
     }

    private void displayError() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        builder.setMessage(getString(R.string.plmn_list_empty));
        builder.setCancelable(true);
        builder.setPositiveButton(getString(android.R.string.ok),
        new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                finish();
            }
        });
        builder.show();
    }

    private void log(String msg) {
        if(DBG) Log.d(LOG_TAG, msg);
    }
}
