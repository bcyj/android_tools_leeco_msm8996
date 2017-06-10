/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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
package com.android.cmcc.customer;

import com.android.cmcc.R;
import com.android.cmcc.service.CmccServer;
import com.android.cmcc.service.Links;
import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class CustomerServiceList extends ListActivity {
    private String[] DATA;
    private String oldName;
    private String oldPhone;
    private String servicePhone;
    private int listType;
    public static final int MYMONTERNET_LIST_TYPE = 1;
    public static final int CUSTOMERSERVICE_LIST_TYPE = 2;

    public CustomerServiceList() {
        this.DATA = new String[1];
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent localIntent = getIntent();
        if (localIntent != null) {
            listType = localIntent.getExtras().getInt("type");
        }
        switch (listType) {
        case MYMONTERNET_LIST_TYPE: {// MY MONTERNET
            getListView().setTextFilterEnabled(true);
            setContentView(R.layout.txt_list_view);
            setTitle(R.string.mymonternet);
            ((TextView) findViewById(R.id.list_view_text))
                    .setText(R.string.mymonternet_txt);
            String[] myMount = new String[3];
            myMount[0] = getString(R.string.mymonternet_wap);
            myMount[1] = getString(R.string.mymonternet_sms);
            myMount[2] = getString(R.string.mymonternet_mms);
            ArrayAdapter<String> myMountentAdapter = new ArrayAdapter<String>(
                    this, R.layout.cmcc_list, myMount);
            setListAdapter(myMountentAdapter);
            break;
        }
        case CUSTOMERSERVICE_LIST_TYPE: {// custom service setting
            this.DATA[0] = getString(R.string.manager_phone);
            setTitle(R.string.setting);
            this.oldName = getIntent().getExtras().getString("managername");
            this.oldPhone = getIntent().getExtras().getString("managerphone");
            ArrayAdapter<String> phoneAdapter = new ArrayAdapter<String>(this,
                    R.layout.cmcc_list, this.DATA);
            setListAdapter(phoneAdapter);
            break;
        }
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        switch (listType) {
        case MYMONTERNET_LIST_TYPE: {
            switch ((int) id) {
            case 0: {
                CmccServer.gotoWapBrowser(this, Links.MYMONTERNET_LINK);
                break;
            }
            case 1: {
                Intent smsIntent = new Intent();
                smsIntent.setClass(CustomerServiceList.this,
                        ServiceInfoView.class);
                smsIntent.putExtra("type", ServiceInfoView.MONTERNET_SMS_TYPE);
                startActivityForResult(smsIntent,
                        ServiceInfoView.MONTERNET_SMS_TYPE);
                break;
            }
            case 2: {
                Intent mmsIntent = new Intent();
                mmsIntent.setClass(CustomerServiceList.this,
                        ServiceInfoView.class);
                mmsIntent.putExtra("type", ServiceInfoView.MONTERNET_MMS_TYPE);
                startActivityForResult(mmsIntent,
                        ServiceInfoView.MONTERNET_MMS_TYPE);
                break;
            }
            }
            break;
        }
        case CUSTOMERSERVICE_LIST_TYPE: {
            switch ((int) id) {
            case 0: {
                Intent intent = new Intent();
                intent.setClass(CustomerServiceList.this, ManagerUpdate.class);
                intent.putExtra("managername", this.oldName);
                intent.putExtra("managerphone", this.oldPhone);
                this.startActivity(intent);
                break;
            }
            }
            break;
        }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        super.onActivityResult(requestCode, resultCode, data);
    }
}
