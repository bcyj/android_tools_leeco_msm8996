/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.salestracker;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class SalesTrackerReceiver extends BroadcastReceiver {
    private static final String TAG = "SalesTracker::Receiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "received intent: " + (action == null ? "null" : action));

        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            int regResult = SalesTrackerService.getRegistrationResult(context);
            Log.d(TAG, "registration result " + regResult);
            if (regResult == SalesTrackerService.REG_SUCCESS) {
                SalesTrackerService.disableReceiver(context);
                return;
            }

            SalesTrackerService.setRegistrationResult(context, SalesTrackerService.REG_UNKNOWN);
            Log.d(TAG, "set registration result to " + SalesTrackerService.REG_UNKNOWN);

            Intent startIntent = new Intent(context, SalesTrackerService.class);
            startIntent.setAction(SalesTrackerService.ACTION_START_SERVICE);
            startIntent.putExtra(SalesTrackerService.EXTRA_TRIGGER_ACTION, action);
            context.startService(startIntent);
        }
        else if (SalesTrackerService.ACTION_SENT_RESULT.equals(action) ||
                SalesTrackerService.ACTION_DELIVERY_RESULT.equals(action)) {
            int retryToken = intent.getIntExtra(
                    SalesTrackerService.EXTRA_RETRY_TOKEN, -1);
            Log.d(TAG, "retry token " + retryToken);
            if (retryToken == -1) {
                return;
            }

            int refNum = intent.getIntExtra(
                    SalesTrackerService.EXTRA_REFERENCE_NUM, -1);
            Log.d(TAG, "reference number " + refNum);
            if (refNum == -1) {
                return;
            }

            int resultCode = getResultCode();
            Log.d(TAG, "result code " + resultCode);

            try {
                SalesTrackerService.sRegResultSemaphore.acquire();
            } catch (InterruptedException e) {
                Log.e(TAG, "acquire exception: " + e);
            }

            int regResult = SalesTrackerService.getRegistrationResult(context);
            Log.d(TAG, "registration result " + regResult);
            if (regResult != SalesTrackerService.REG_UNKNOWN) {
                SalesTrackerService.sRegResultSemaphore.release();
                return;
            }

            Intent startIntent = new Intent(context, SalesTrackerService.class);
            startIntent.setAction(SalesTrackerService.ACTION_START_SERVICE);
            startIntent.putExtra(SalesTrackerService.EXTRA_TRIGGER_ACTION, action);
            startIntent.putExtra(SalesTrackerService.EXTRA_RETRY_TOKEN, retryToken);
            startIntent.putExtra(SalesTrackerService.EXTRA_REFERENCE_NUM, refNum);
            startIntent.putExtra(SalesTrackerService.EXTRA_RESULT_CODE, resultCode);
            context.startService(startIntent);
        }
    }
}
