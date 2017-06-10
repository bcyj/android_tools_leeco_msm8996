/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess;

import static com.qapp.secprotect.utils.UtilsLog.logd;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class RequestActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        logd();
        Intent intent = getIntent();
        if (intent == null) {
            finish();
            return;
        }

        intent.addFlags(Intent.FLAG_ACTIVITY_PREVIOUS_IS_TOP
                | Intent.FLAG_ACTIVITY_NO_ANIMATION);
        String name = getClass().getPackage().getName() + "."
                + PromptActivity.class.getSimpleName();
        intent.setClassName(this, name);
        startActivity(intent);
        finish();
        return;
    }
}
