/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.xdivert;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import java.util.List;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

// SIM records have msisdn.Hence directly process XDivert feature

public class XDivertSetting extends TimeConsumingPreferenceActivity {
    private static final String LOG_TAG = "XDivertSetting";

    private static final String BUTTON_XDIVERT = "xdivert_checkbox";

    private XDivertCheckBoxPreference mXDivertButton;
    public static final int LINE_NUMBERS = 1;
    private int mNumPhones;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.xdivert);
        mNumPhones = TelephonyManager.getDefault().getPhoneCount();
        processXDivert();
   }

    private boolean isSlotActive(int slotId) {
        boolean slotActive = false;
        List<SubscriptionInfo> activeSubList =
                SubscriptionManager.from(this).getActiveSubscriptionInfoList();
        if (activeSubList != null) {
            for (SubscriptionInfo subScriptionInfo : activeSubList) {
                if (subScriptionInfo.getSimSlotIndex() == slotId) {
                    slotActive = true;
                     break;
                }
            }
        }
        return slotActive;
    }

    private boolean isAllSubActive() {
        for (int i = 0; i < mNumPhones; i++) {
            if (!isSlotActive(i)) return false;
        }
        return true;
    }

    private boolean isAnySubCdma() {
        for (int i = 0; i < mNumPhones; i++) {
            Phone phone = PhoneFactory.getPhone(i);
            if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) return true;
        }
        return false;
    }

    private boolean isValidLine1Number(String[] line1Numbers) {
        for (int i = 0; i < mNumPhones; i++) {
            if (TextUtils.isEmpty(line1Numbers[i])) return false;
        }
        return true;
    }

    private void processXDivert() {
        String[] line1Numbers = new String[mNumPhones];
        for (int i = 0; i < mNumPhones; i++) {
            Phone phone = PhoneFactory.getPhone(i);
            String msisdn = phone.getLine1Number();  // may be null or empty
            if (!TextUtils.isEmpty(msisdn)) {
                //Populate the line1Numbers only if it is not null
               line1Numbers[i] = PhoneNumberUtils.formatNumber(msisdn);
            }

            Log.d(LOG_TAG, "SUB:" + i + " phonetype = " + phone.getPhoneType()
                    + " isSlotActive = " + isSlotActive(i)
                    + " line1Number = " + line1Numbers[i]);
        }
        if (!isAllSubActive()) {
            //Is a subscription is deactived/or only one SIM is present,
            //dialog would be displayed stating the same.
            displayAlertDialog(R.string.xdivert_sub_absent);
        } else if (isAnySubCdma()) {
            //X-Divert is not supported for CDMA phone.Hence for C+G / C+C,
            //dialog would be displayed stating the same.
            displayAlertDialog(R.string.xdivert_not_supported);
        } else if (!isValidLine1Number(line1Numbers)) {
            //SIM records does not have msisdn, hence ask user to enter
            //the phone numbers.
            Intent intent = new Intent();
            intent.setClass(this, XDivertPhoneNumbers.class);
            startActivityForResult(intent, LINE_NUMBERS);
        } else {
            //SIM records have msisdn.Hence directly process
            //XDivert feature
            processXDivertCheckBox(line1Numbers);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
         Log.d(LOG_TAG, "requestCode: "+ requestCode + " resultCode: " + resultCode);
        if (requestCode == LINE_NUMBERS) {
            if (resultCode == RESULT_OK) {
                String[] numbers  = data.getStringArrayExtra(XDivertUtility.LINE1_NUMBERS);
                Log.d(LOG_TAG, "numbers: "+ numbers);
                processXDivertCheckBox(numbers);
            } else {
                finish();
            }
        }
    }

    private void displayAlertDialog(int resId) {
        new AlertDialog.Builder(this).setMessage(resId)
            .setTitle(R.string.xdivert_title)
            .setIcon(android.R.drawable.ic_dialog_alert)
            .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(LOG_TAG, "X-Divert onClick");
                        finish();
                    }
                })
            .show()
            .setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        Log.d(LOG_TAG, "X-Divert onDismiss");
                        finish();
                    }
            });
    }

    private void processXDivertCheckBox(String[] line1Numbers) {
        Log.d(LOG_TAG,"processXDivertCheckBox line1Numbers = "
                + java.util.Arrays.toString(line1Numbers));
        PreferenceScreen prefSet = getPreferenceScreen();
        mXDivertButton = (XDivertCheckBoxPreference) prefSet.findPreference(BUTTON_XDIVERT);
        mXDivertButton.init(this, false, line1Numbers);
    }


}
