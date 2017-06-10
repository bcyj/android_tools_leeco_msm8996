/**
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package qualcomm.android.LEDFlashlight;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class LEDGestureActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sendBroadcast(new Intent(LEDWidget.broadCastString));
        finish();
    }

}
