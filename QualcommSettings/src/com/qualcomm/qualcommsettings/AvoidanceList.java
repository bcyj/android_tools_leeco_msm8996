/******************************************************************************
 * @file    AvoidanceList.java
 * @brief   Displays the system avoidance list for CDMA network
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/
package com.qualcomm.qualcommsettings;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class AvoidanceList extends Activity {

    private ListView listView;
    private static final String TAG = "CdmaNwkAvoidance";
    private static final int MAX_AVOIDANCE_NWKS = 10;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.avoid_list);

        listView = (ListView) findViewById(R.id.listview);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            byte[] data = extras.getByteArray("avoidance_list");
            if (data != null) {
                ByteBuffer buffer = ByteBuffer.wrap(data);
                buffer.order(ByteOrder.LITTLE_ENDIAN);
                int item_num = buffer.getInt();
                if (item_num <= MAX_AVOIDANCE_NWKS && item_num > 0) {
                    ListItem[] items = new ListItem[item_num];

                    // Build the items array
                    for (int i = 0; i < item_num; i++) {
                        items[i] = new ListItem(buffer.getInt(), buffer.getInt(),
                                buffer.getInt(), buffer.getInt());
                    }

                    AvoidListItemAdapter adapter = new AvoidListItemAdapter(this,
                            R.layout.avoid_list_item, items);
                    listView.setAdapter(adapter);
                } else {
                    Log.d(TAG, "Invaild item number in the AvoidanceList:" + item_num);
                }
            } else {
                Log.e(TAG, "AvoidanceList did not find avoidance_list in Extras");
            }
        } else {
            Log.e(TAG, "AvoidanceList getExtras returned null");
        }
    }

    private final class AvoidListItemAdapter extends ArrayAdapter<ListItem> {
        private Context mContext;
        private LayoutInflater mInflater;

        public AvoidListItemAdapter(Context context, int textViewResourceId, ListItem[] obj) {
            super(context, textViewResourceId, obj);

            mContext = context;
            mInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if(convertView == null){
                convertView = mInflater.inflate(R.layout.avoid_list_item, null);
            }

            ListItem listItem = getItem(position);

            ((TextView) convertView.findViewById(R.id.sid)).setText(listItem.get_sid());
            ((TextView) convertView.findViewById(R.id.nid)).setText(listItem.get_nid());
            ((TextView) convertView.findViewById(R.id.mnc)).setText(listItem.get_mnc());
            ((TextView) convertView.findViewById(R.id.mcc)).setText(listItem.get_mcc());

            return convertView;
        }
    }

    public final class ListItem {
        private String sid;
        private String nid;
        private String mcc;
        private String mnc;

        public String get_sid() {
            return sid;
        }

        public String get_nid() {
            return nid;
        }

        public String get_mcc() {
            return mcc;
        }

        public String get_mnc() {
            return mnc;
        }

        public ListItem(int sid, int nid, int mnc, int mcc) {
            this.sid = String.valueOf(sid);
            this.nid = String.valueOf(nid);
            this.mnc = String.valueOf(mnc);
            this.mcc = String.valueOf(mcc);
        }
    }
}

