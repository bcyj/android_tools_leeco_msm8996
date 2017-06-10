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

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class CustomerService extends ListActivity {

    public static final String MANAGER_PREFS_NAME = "managerinfo";
    public static final String SERVICEPHONE_PREFS_NAME = "servicephone";

    public static final int SERVICE_GUIDE_REQUEST_CODE = 0;
    public static final int SERVICE_PHONE_REQUEST_CODE = 1;
    public static final int MYMONTERNET_SMS_REQUEST_CODE = 2;
    public static final int MYMONTERNET_MMS_REQUEST_CODE = 3;
    public static final int CUSTOMER_MANAGER_REQUEST_CODE = 4;
    public static final int SETTING_SERVICEPHONE_REQUEST_CODE = 5;

    public static boolean radioOn = false;

    private String[] mDatas;
    private MyPhoneStateListener mPhoneStateListener;
    private TelephonyManager mTelephonyManager;
    private Toast mToast;

    public CustomerService() {
        this.mDatas = new String[6];
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("dsfsdfsdf", "sdfasdfasdfasdf");
        TelephonyManager localTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        this.mTelephonyManager = localTelephonyManager;
        registerPhoneStateListener();
        this.mDatas[0] = getString(R.string.service_guide);
        this.mDatas[1] = getString(R.string.service_phone);
        this.mDatas[2] = getString(R.string.sms_service);
        this.mDatas[3] = getString(R.string.wap_service);
        this.mDatas[4] = getString(R.string.customer_manager);
        this.mDatas[5] = getString(R.string.setting);
        setTitle(R.string.list_customerservice_menu);
        EfficientAdapter localEfficientAdapter = new EfficientAdapter(this,
                this.mDatas);
        setListAdapter(localEfficientAdapter);
    }

    public void registerPhoneStateListener() {
        if (this.mPhoneStateListener == null) {
            MyPhoneStateListener localMyPhoneStateListener1 = new MyPhoneStateListener(
                    this, null);
            this.mPhoneStateListener = localMyPhoneStateListener1;
        }
        TelephonyManager localTelephonyManager = this.mTelephonyManager;
        MyPhoneStateListener localMyPhoneStateListener2 = this.mPhoneStateListener;
        localTelephonyManager.listen(localMyPhoneStateListener2, 1);
    }

    class MyPhoneStateListener extends PhoneStateListener {
        public MyPhoneStateListener(CustomerService customerService,
                Object object) {
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            String str = "onServiceStateChanged, state=" + serviceState;
            if (serviceState.getState() == serviceState.STATE_IN_SERVICE) {
                CustomerService.radioOn = true;
            } else {
                CustomerService.radioOn = false;
            }
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        switch ((int) id) {
        case SERVICE_GUIDE_REQUEST_CODE: {
            Intent intent = new Intent();
            intent.setClass(CustomerService.this, ServiceInfoView.class);
            intent.putExtra("type", ServiceInfoView.SERCICE_GUIDE_TYPE);
            int i = 0;
            this.startActivityForResult(intent, i);
            break;
        }
        case SERVICE_PHONE_REQUEST_CODE: {
            SharedPreferences sp = this.getSharedPreferences("servicephone", 0);
            String phoneNum = sp.getString("phonenumber", "10086");
            Uri uri = Uri.parse("tel:" + phoneNum);
            Intent intent = new Intent("android.intent.action.CALL", uri);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
            this.startActivity(intent);
            break;
        }
        case MYMONTERNET_SMS_REQUEST_CODE: {
            try {
                StringBuilder localStringBuilder1 = new StringBuilder(
                        "radioOn is ");
                boolean bool = radioOn;
                Log.i("CustomerService", "#####" + bool);
                if ((!(radioOn)) || (!(this.mTelephonyManager.hasIccCard()))) {
                    Log.i("CustomerService",
                            "#####!RadioOn" + !radioOn + " ##########"
                                    + !(this.mTelephonyManager.hasIccCard()));
                    Toast.makeText(this, R.string.sms_no_network,
                            Toast.LENGTH_SHORT).show();
                } else {
                    SmsManager.getDefault().sendTextMessage("10086", null,
                            "10086", null, null);
                    Toast.makeText(this, R.string.sms_sending,
                            Toast.LENGTH_SHORT).show();
                }
            } catch (Exception localException) {
                Toast.makeText(this, R.string.sms_no_network,
                        Toast.LENGTH_SHORT).show();
            }
            break;
        }
        case MYMONTERNET_MMS_REQUEST_CODE: {
            String http = "http://wap.monternet.com/?cp22=v22yyt";
            CmccServer.gotoWapBrowser(this, http);
            break;
        }
        case CUSTOMER_MANAGER_REQUEST_CODE: {
            SharedPreferences sp = this.getSharedPreferences("managerinfo", 0);
            String phone = sp.getString("managerphone", "");
            String name = sp.getString("managername", "");
            if (phone.length() <= 0) {
                Intent intent = new Intent();
                intent.setClass(CustomerService.this, ManagerUpdate.class);
                intent.putExtra("managername", name);
                intent.putExtra("managerphone", phone);
                this.startActivityForResult(intent, 9);
            } else {
                phone = phone.replace('P', ',');
                phone = phone.replace('W', ';');
                phone = phone.replace('p', ',');
                phone = phone.replace('w', ';');
                Uri uri = Uri.fromParts("tel", phone, null);
                Log.d("CustomerService", "wxl  uri=" + uri);

                Intent intent = new Intent("android.intent.action.CALL", uri);
                intent.putExtra("android.intent.extra.UID", name);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
                this.startActivity(intent);
            }

            break;
        }
        case SETTING_SERVICEPHONE_REQUEST_CODE: {
            Intent intent = new Intent();
            SharedPreferences sp = this.getSharedPreferences("managerinfo", 0);
            String phone = sp.getString("managerphone", "");
            String name = sp.getString("managername", "");
            String servicePhone = sp.getString("servicephone", "");
            intent.setClass(CustomerService.this, CustomerServiceList.class);
            intent.putExtra("managername", name);
            intent.putExtra("managerphone", phone);
            intent.putExtra("type",
                    CustomerServiceList.CUSTOMERSERVICE_LIST_TYPE);
            this.startActivityForResult(intent, 9);
            break;
        }
        default: {

        }
        }
    }

    class EfficientAdapter extends BaseAdapter {
        private String[] ListLabels;
        private Context mContext;
        private LayoutInflater mInflater;

        public EfficientAdapter(CustomerService customerService, String[] data) {
            LayoutInflater localLayoutInflater = (LayoutInflater) customerService
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            this.mInflater = localLayoutInflater;
            this.mContext = customerService;
            this.ListLabels = data;
        }

        public int getCount() {
            return this.ListLabels.length;
        }

        public Object getItem(int arg0) {
            return Integer.valueOf(arg0);
        }

        public long getItemId(int id) {
            return id;
        }

        public View getView(int pos, View view, ViewGroup viewGroup) {
            ViewHolder localViewHolder;
            Log.i("CMCCService", "--view==null---");
            Context localContext = this.mContext;
            view = new LinearLayout(localContext);
            LayoutInflater localLayoutInflater = this.mInflater;
            LinearLayout localLinearLayout = (LinearLayout) view;
            localLayoutInflater.inflate(R.layout.customer_service_list,
                    localLinearLayout);
            localViewHolder = new ViewHolder();
            TextView txt1 = (TextView) view.findViewById(R.id.server_list_text);
            txt1.setText(this.ListLabels[pos]);
            localViewHolder.text = txt1;
            view.setTag(localViewHolder);
            return view;
        }

        class ViewHolder {
            ImageView icon;
            TextView text;
        }
    }
}
