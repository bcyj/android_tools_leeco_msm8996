/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package com.qualcomm.ims.csvt;

import android.os.SystemProperties;
import android.util.Log;

import com.android.ims.ImsSsInfo;
import com.android.ims.ImsCallForwardInfo;
import com.android.ims.ImsUtInterface;
import com.android.internal.telephony.CommandsInterface;

import org.codeaurora.ims.csvt.CallForwardInfoP;

import java.util.ArrayList;

public class CsvtUtils {

    private static final String TAG = "CsvtUtils";
    private static final String CSVT_PROP = "persist.radio.csvt.enabled";

    public static boolean isCsvtEnabled() {
        return SystemProperties.getBoolean(CSVT_PROP, false);
    }

    public static int getUtConditionFromCFReason(int reason) {
        int condition = ImsUtInterface.INVALID;
        switch (reason) {
            case CommandsInterface.CF_REASON_UNCONDITIONAL:
                condition = ImsUtInterface.CDIV_CF_UNCONDITIONAL;
                break;
            case CommandsInterface.CF_REASON_BUSY:
                condition = ImsUtInterface.CDIV_CF_BUSY;
                break;
            case CommandsInterface.CF_REASON_NO_REPLY:
                condition = ImsUtInterface.CDIV_CF_NO_REPLY;
                break;
            case CommandsInterface.CF_REASON_NOT_REACHABLE:
                condition = ImsUtInterface.CDIV_CF_NOT_REACHABLE;
                break;
            case CommandsInterface.CF_REASON_ALL:
                condition = ImsUtInterface.CDIV_CF_ALL;
                break;
            case CommandsInterface.CF_REASON_ALL_CONDITIONAL:
                condition = ImsUtInterface.CDIV_CF_ALL_CONDITIONAL;
                break;
            default:
                break;
        }
        return condition;
    }

    public static int getCfReasonFromUtCondition(int condition) {
        int reason = CommandsInterface.CF_REASON_NOT_REACHABLE;
        switch (condition) {
            case ImsUtInterface.CDIV_CF_UNCONDITIONAL:
                reason = CommandsInterface.CF_REASON_UNCONDITIONAL;
                break;
            case ImsUtInterface.CDIV_CF_BUSY:
                reason =  CommandsInterface.CF_REASON_BUSY;
                break;
            case ImsUtInterface.CDIV_CF_NO_REPLY:
                reason =  CommandsInterface.CF_REASON_NO_REPLY;
                break;
            case ImsUtInterface.CDIV_CF_NOT_REACHABLE:
                reason =  CommandsInterface.CF_REASON_NOT_REACHABLE;
                break;
            case ImsUtInterface.CDIV_CF_ALL:
                reason =  CommandsInterface.CF_REASON_ALL;
                break;
            case ImsUtInterface.CDIV_CF_ALL_CONDITIONAL:
                reason =  CommandsInterface.CF_REASON_ALL_CONDITIONAL;
                break;
            default:
                break;
        }
        return reason;
    }

    public static int getUtActionFromCFAction(int action) {
        int utAction = ImsUtInterface.INVALID;
        switch(action) {
            case CommandsInterface.CF_ACTION_DISABLE:
                utAction = ImsUtInterface.ACTION_DEACTIVATION;
                break;
            case CommandsInterface.CF_ACTION_ENABLE:
                utAction = ImsUtInterface.ACTION_ACTIVATION;
                break;
            case CommandsInterface.CF_ACTION_ERASURE:
                utAction = ImsUtInterface.ACTION_ERASURE;
                break;
            case CommandsInterface.CF_ACTION_REGISTRATION:
                utAction = ImsUtInterface.ACTION_REGISTRATION;
                break;
            default:
                break;
        }
        return utAction;
    }

    public static boolean getCwEnabledFromUtResult(Object result) {
        boolean enabled = false;
        if (result != null && result instanceof ImsSsInfo[]) {
            ImsSsInfo[] cwInfo = (ImsSsInfo[]) result;
            if (cwInfo.length > 0 && cwInfo[0] != null) {
                enabled = (cwInfo[0].mStatus == ImsSsInfo.ENABLED);
                Log.d(TAG, "getCwEnabledFromUtResult: enabled = " + enabled);
            }
        }
        return enabled;
    }

    public static ArrayList<CallForwardInfoP> getCfInfoFromUtResult(Object result) {
        ArrayList<CallForwardInfoP> cfInfoPArr = new ArrayList<CallForwardInfoP>();
        if (result != null && result instanceof ImsCallForwardInfo[]) {
            ImsCallForwardInfo[] cfInfoArray = (ImsCallForwardInfo[]) result;
            Log.d(TAG, "handleGetCFResponse: cfInfoArray.length = " + cfInfoArray.length);
            for (int i = 0, length = cfInfoArray.length; i < length; i++) {
                Log.d(TAG, "handleGetCFResponse, cfInfoArray[" + i + "]="
                        + cfInfoArray[i]);
                CallForwardInfoP cf = new CallForwardInfoP();
                cf.status = cfInfoArray[i].mStatus;
                cf.reason = getCfReasonFromUtCondition(cfInfoArray[i].mCondition);
                cf.toa = cfInfoArray[i].mToA;
                cf.number = cfInfoArray[i].mNumber;
                cf.timeSeconds = cfInfoArray[i].mTimeSeconds;
                cfInfoPArr.add(cf);
            }
        }
        return cfInfoPArr;
    }
}
