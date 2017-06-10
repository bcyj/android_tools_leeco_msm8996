/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.wdstechnology.android.kryten;

import android.app.Activity;
import android.content.Intent;
import android.view.View;
import android.widget.Button;

import com.wdstechnology.android.kryten.utils.HexConvertor;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

public class KrytenActivityLauncher extends Activity {

    protected static final String TEST_DOCUMENT = "030B6A0B566620495350007765620045C65601870706"
            + "83000101C65501871106830001870706830001871006AB0187080603696E7465726E65740001870906"
            + "8901C65A01870C069A01870D06830701870E068307010101C65701870706034E4554574F524B494445"
            + "4E5400018705034D434300060332333400018705034D4E430006033135000101C60001550187360000"
            + "060377320001870706830001872206830001C600015901873A00000603687474703A2F2F6C6976652E"
            + "766F6461666F6E652E636F6D000187070603566F6461666F6E650001871C01010101";

    @Override
    protected void onStart() {
        super.onStart();
        setContentView(R.layout.kryten_activity_launcher);

        Button notifyUserPin = (Button) findViewById(R.id.send_pdu_to_notifier_user_pin);
        notifyUserPin.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                Intent intent = new Intent(KrytenActivityLauncher.this,
                        ProvisioningPushReceiver.class);
                Map<String, String> params = new HashMap<String, String>();

                params.put("SEC", "1");
                params.put("MAC", "FA8B0A2DE72948800CA24119DFA619AE08CBFE44");

                intent.putExtra("data", HexConvertor.convert(TEST_DOCUMENT));

                intent.putExtra("contentTypeParameters", (Serializable) params);

                KrytenActivityLauncher.this.sendBroadcast(intent);

            }
        });

        Button confirm = (Button) findViewById(R.id.straight_to_confirm);
        confirm.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                Intent intent = new Intent(KrytenActivityLauncher.this, StoreProvisioning.class);

                intent.putExtra("com.wdstechnology.android.kryten.provisioning-data", HexConvertor
                        .convert(TEST_DOCUMENT));

                startActivity(intent);

            }
        });

        Button error = (Button) findViewById(R.id.straight_to_error);
        error.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                ProvisioningFailed.fail(KrytenActivityLauncher.this,
                        R.string.bad_network_pin_message);
            }
        });

    }

}
