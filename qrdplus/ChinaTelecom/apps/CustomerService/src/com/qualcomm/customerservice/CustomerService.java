/**
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.customerservice;

import android.telephony.MSimTelephonyManager;
import android.telephony.TelephonyManager;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.SimpleAdapter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import com.qualcomm.customerservice.R;

public class CustomerService extends Activity {
    private final String TAG = "CTCustomerService";
    private List<Map<String, Object>> mList;
    private Intent mIntent;
    private SimpleAdapter mAdapter;
    ListView mListView = null;
    private MsmUtil mMsmutil = null;
    private static final String VIEWACTION = "view.action";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.v(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        mIntent = this.getIntent();
        getAdapter();
        if (mListView != null)
            mListView.setAdapter(mAdapter);
        mMsmutil = new MsmUtil(this);
        if (!isUIMInsert()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage(R.string.noUSIM);
            builder.setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            builder.setCancelable(false).create().show();
        }
    }

    @Override
    protected void onDestroy() {
        Log.v(TAG, "onDestroy");
        super.onDestroy();
        mMsmutil.release();
    }

    private void getAdapter() {
        TextView title;
        String action = mIntent.getStringExtra(VIEWACTION);
        Log.d(TAG, "action = " + action);

        if ((action == null) || "".equals(action)) {
            mList = getTopData();
            setContentView(R.layout.list);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(R.string.app_name));
            mListView = (ListView) this.findViewById(R.id.list);
            mAdapter = new SimpleAdapter(this, mList, R.layout.list_item,
                    new String[] { "ItemText" }, new int[] { R.id.ItemText });
            mListView.setOnItemClickListener(getTopLevelViewListListen());
            return;
        }

        if (action.equals(String.valueOf(R.string.customer_service_hotline))) {
            // Customer Service Hot line
            setContentView(R.layout.customer_service_hotline);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(Integer.valueOf(action)));

            TextView tv = (TextView) this.findViewById(R.id.displaytext);
            tv.setText(getResources().getString(
                    R.string.customer_service_hotline_content));
            mListView = (ListView) this.findViewById(R.id.list);
            mList = getCSHotLineData();
            mAdapter = new SimpleAdapter(this, mList, R.layout.list_item,
                    new String[] { "ItemText" }, new int[] { R.id.ItemText });
            mListView.setOnItemClickListener(getCSHotLineViewListen());
            return;
        }

        // palm business hall
        if (action.equals(String.valueOf(R.string.palm_business_hall))) {
            Log.d(TAG, "palm business hall list view");
            mList = getPBHallData();
            setContentView(R.layout.list);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(Integer.valueOf(action)));

            mListView = (ListView) this.findViewById(R.id.list);
            mAdapter = new SimpleAdapter(this, mList, R.layout.list_item,
                    new String[] { "ItemText" }, new int[] { R.id.ItemText });
            mListView.setOnItemClickListener(getPBHallViewListen());
            return;
        }

        // business inquire unsubscribe
        if (action
                .equals(String.valueOf(R.string.business_inquire_unsubscribe))) {
            mList = getBIData();
            setContentView(R.layout.list);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(Integer.valueOf(action)));

            mListView = (ListView) this.findViewById(R.id.list);
            mAdapter = new SimpleAdapter(this, mList, R.layout.list_item,
                    new String[] { "ItemText" }, new int[] { R.id.ItemText });
            mListView.setOnItemClickListener(getBIViewListen());
            return;
        }

        // international roaming services
        if (action.equals(String
                .valueOf(R.string.international_roaming_service))) {
            setContentView(R.layout.international_roaming);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(Integer.valueOf(action)));

            TextView tv = (TextView) this.findViewById(R.id.globaltext);
            tv.setMovementMethod(LinkMovementMethod.getInstance());
            return;
        }

        // mobile phone service guide
        if (action.equals(String.valueOf(R.string.mobile_service_guide))) {
            setContentView(R.layout.mobile_service_guide);
            getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                    R.layout.title);
            title = (TextView) this.findViewById(R.id.title);
            title.setText(getResources().getString(Integer.valueOf(action)));
            return;
        }
    }

    private List<Map<String, Object>> getTopData() {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();

        Map<String, Object> map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.palm_business_hall));
        map.put(this.VIEWACTION, String.valueOf(R.string.palm_business_hall));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.business_inquire_unsubscribe));
        map.put(this.VIEWACTION,
                String.valueOf(R.string.business_inquire_unsubscribe));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.roaming_list_update));
        map.put(this.VIEWACTION, String.valueOf(R.string.roaming_list_update));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.customer_service_hotline));
        map.put(this.VIEWACTION,
                String.valueOf(R.string.customer_service_hotline));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources()
                        .getString(R.string.international_roaming_service));
        map.put(this.VIEWACTION,
                String.valueOf(R.string.international_roaming_service));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.mobile_service_guide));
        map.put(this.VIEWACTION, String.valueOf(R.string.mobile_service_guide));
        list.add(map);

        return list;
    }

    // Get Customer Service Hot line
    private List<Map<String, Object>> getCSHotLineData() {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        String[] Items = getResources().getStringArray(
                R.array.customer_service_hotlines);
        for (String hotlineItem : Items) {
            Log.d(TAG, "hotlineItem = " + hotlineItem);
            Map<String, Object> map = new HashMap<String, Object>();
            map.put("ItemText", hotlineItem);
            list.add(map);
        }

        return list;
    }

    // get palm business hall data
    private List<Map<String, Object>> getPBHallData() {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        Map<String, Object> map = new HashMap<String, Object>();
        map.put("ItemText", getResources().getString(R.string.hall_message));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText", getResources().getString(R.string.hall_wap));
        list.add(map);

        return list;
    }

    // business inquire unsubscribe
    private List<Map<String, Object>> getBIData() {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        Map<String, Object> map = new HashMap<String, Object>();
        map.put("ItemText", getResources().getString(R.string.business_inquire));
        list.add(map);

        map = new HashMap<String, Object>();
        map.put("ItemText",
                getResources().getString(R.string.business_unsubscribe));
        list.add(map);

        return list;

    }

    private OnItemClickListener getTopLevelViewListListen() {
        return (new OnItemClickListener() {
            public void onItemClick(AdapterView<?> l, View v, int position,
                    long id) {
                String itemText = (String) mList.get(position).get(VIEWACTION);
                // click remoting list update item
                if (itemText.equals(String
                        .valueOf(R.string.roaming_list_update))) {
                    mMsmutil.sendMsmDelay(CustomerService.this, "10659165",
                            "PRL");
                    return;
                }

                Log.d(TAG, "onListItemClick.itemText=" + itemText);
                Intent intent = new Intent(CustomerService.this,
                        CustomerService.class);
                intent.putExtra(VIEWACTION, itemText);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            }
        });
    }

    private OnItemClickListener getCSHotLineViewListen() {
        return (new OnItemClickListener() {
            public void onItemClick(AdapterView<?> l, View v, int position,
                    long id) {
                String itemText = (String) mList.get(position).get("ItemText");
                Log.d(TAG, "onListItemClick.itemText=" + itemText);
                Intent intent = new Intent();
                intent.setAction(Intent.ACTION_CALL);
                intent.setData(Uri.parse("tel:" + itemText));
                startActivity(intent);
            }
        });
    }

    // get palm business hall data
    private OnItemClickListener getPBHallViewListen() {
        return (new OnItemClickListener() {
            public void onItemClick(AdapterView<?> l, View v, int position,
                    long id) {
                String itemText = (String) mList.get(position).get("ItemText");
                Log.d(TAG, "getPBHallViewListen. itemText=" + itemText);
                if (itemText.equals(getResources().getString(
                        R.string.hall_message))) {
                    // send 10001 to 10001
                    mMsmutil.sendMsmDelay(CustomerService.this, "10001",
                            "10001");
                } else {
                    Uri uri = Uri.parse(getResources().getString(
                            R.string.ctwapNet));
                    Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                    startActivity(intent);
                }
            }
        });
    }

    // get business inquire listen
    private OnItemClickListener getBIViewListen() {
        return (new OnItemClickListener() {
            public void onItemClick(AdapterView<?> l, View v, int position,
                    long id) {
                String itemText = (String) mList.get(position).get("ItemText");
                String texts;
                if (itemText.equals(getResources().getString(
                        R.string.business_inquire))) {
                    // send 0000 to 10001
                    texts = "0000";
                } else {
                    // send 0000 to 10001
                    texts = "00000";
                }
                mMsmutil.sendMsmDelay(CustomerService.this, "10001", texts);
            }
        });
    }

    private boolean isUIMInsert() {
        if (MSimTelephonyManager.getDefault().isMultiSimEnabled()) {
            return MSimTelephonyManager.getDefault().hasIccCard(0);
        } else {
            return TelephonyManager.getDefault().hasIccCard();
        }
    }
}
