/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
// import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;

public class QWorldClock extends Activity implements OnClickListener, OnItemClickListener, OnItemLongClickListener {
//    private static final String TAG = "QWorldClock";
    public static final String CITY = "city";
    public static final String TIME_ZONE = "ID";
    public static final String ANALOG_CLOCK = "clock_analog";
    public static final String DIGITAL_CLOCK = "clock_digital";
    final static String ACTION = "Action";
    final static String ACTION_ADD_TIME_ZONE = "add";
    final static String ACTION_SELECT_TIME_ZONE = "select";
    final static String ITEM_POSITION = "position";
    final static String _ID = "_id";  // remember the _id of the item in database table in order to delete/update the item
    final static int RESULT_ADD_TIME_ZONE = 0x11;
    final static int RESULT_SELECT_TIME_ZONE = 0x12;
    final static int MAX_CLOCK_NUMBER = 15;

    private ArrayList<HashMap<String, Object>> mListItem = new ArrayList<HashMap<String, Object>>();
    private DatabaseOperator mDb = null;
    private TickBroadCastor mBroadCastor = null;

    private ListView mListView = null;
    private BaseAdapter mAdapter = null;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Button btnAddTimeZone = (Button) findViewById(R.id.btn_add_time_zone);
        btnAddTimeZone.setOnClickListener(this);

        mListView = (ListView) findViewById(R.id.lv_records);
        mListView.setOnItemClickListener(this);
        mAdapter = new ItemAdapter(this);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemLongClickListener(this);

        loadFromDatabase(); // init mListItem from database

        mBroadCastor = TickBroadCastor.getInstance();
    }

    @Override
    public void onResume() {
        super.onResume();
        mBroadCastor.start();
    }

    @Override
    public void onPause() {
        super.onPause();
        mBroadCastor.stop();
    }

    class ItemAdapter extends BaseAdapter {
        private final Context mContext;
        private LayoutInflater mInflater = null;

        ItemAdapter(Context context) {
            mContext = context;
            mInflater = LayoutInflater.from(mContext);
        }

        public int getCount() {
            return mListItem.size();
        }

        public Object getItem(int position) {
            return mListItem.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        @SuppressWarnings("unchecked")
        public View getView(int position, View convertView, ViewGroup parent) {
            if(null == convertView) {
                convertView = mInflater.inflate(R.layout.clock_list, null);
            }
            HashMap<String, Object> map = (HashMap<String, Object>) getItem (position);
            boolean isCustomTimezone= map.get(TimeZoneList.KEY_CUSTOM).equals("TRUE");
            String city = null;
            String timezone = null;
            if(isCustomTimezone) {
                city = (String)(map.get(TimeZoneList.KEY_CITY));
                timezone = (String)(map.get(TimeZoneList.KEY_GMT));
            } else {
                int sequence = Integer.parseInt((String)map.get(TimeZoneList.KEY_SEQUENCE));
                city = (getResources().getStringArray(R.array.city_names))[ sequence - 1];
                timezone = (String)(map.get(TimeZoneList.KEY_TIMEZONE_ID));
            }
            city += "\n" + (String)(map.get(TimeZoneList.KEY_GMT));
            TextView tv = (TextView) convertView.findViewById(R.id.city);
            tv.setTextSize(15);
            tv.setSingleLine(false);
            tv.setText(city);

            QDigitalClock clockDigital = (QDigitalClock) convertView.findViewById(R.id.clock_digital);
            clockDigital.setTimeZone(timezone);
            clockDigital.setContext(mContext);

            QAnalogClock clockAnalog = (QAnalogClock) convertView.findViewById(R.id.clock);
            clockAnalog.setTimeZone(timezone);
            return convertView;
        }
    }

    public void onClick(View v) {
        switch(v.getId()) {
        case R.id.btn_add_time_zone:
            if(mListItem.size() >= MAX_CLOCK_NUMBER) {
                Toast.makeText(this, R.string.clock_reach_max, Toast.LENGTH_SHORT).show();
            } else {
                Intent intent = new Intent(this, TimeZoneList.class);
                // Add flag Intent.FLAG_ACTIVITY_CLEAR_TOP to prevent
                // creating more than one instance when clicks too fast.
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                Bundle bundle = new Bundle();
                bundle.putString(ACTION, ACTION_ADD_TIME_ZONE);
                intent.putExtras(bundle);
                startActivityForResult(intent, RESULT_ADD_TIME_ZONE);
            }
            break;
        default:
            break;
        }
    }

    public void onItemClick(AdapterView<?> adapter, View item, int position, long id) {
        Intent intent = new Intent(this, TimeZoneList.class);
        Bundle bundle = new Bundle();
        bundle.putString(ACTION, ACTION_SELECT_TIME_ZONE);
        bundle.putLong(ITEM_POSITION, id);
        intent.putExtras(bundle);
        startActivityForResult(intent, RESULT_SELECT_TIME_ZONE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Bundle bundle = null;
        switch(resultCode) {
        case RESULT_ADD_TIME_ZONE:
            bundle = data.getExtras();
            if(bundle.getBoolean(TimeZoneList.KEY_CUSTOM, false)) {
                // customer timezone
                addTimeZone(TimeZoneList.SEQUENCE_CUSTOMER,  // sequence in the timezone list
                        bundle.getString(TimeZoneList.KEY_CITY), // city name
                        "", // Timezone id
                        bundle.getString(TimeZoneList.KEY_GMT), // GMT
                        true); // custom
            } else {
                // add from the list
                addTimeZone(bundle.getInt(TimeZoneList.KEY_SEQUENCE, TimeZoneList.SEQUENCE_INVALID), // sequence
                        "", // city name
                        bundle.getString(TimeZoneList.KEY_TIMEZONE_ID), // Timezone id
                        bundle.getString(TimeZoneList.KEY_GMT), // GMT
                        false); // custom
            }
            break;
        case RESULT_SELECT_TIME_ZONE:
            bundle = data.getExtras();
            if(bundle.getBoolean(TimeZoneList.KEY_CUSTOM, false)) {
                // replaced by customer timezone
                changeItem(bundle.getLong(ITEM_POSITION), // position index in the list view
                    TimeZoneList.SEQUENCE_CUSTOMER,  // sequence
                    bundle.getString(TimeZoneList.KEY_CITY), // city name
                    "", //  Tiemzone id
                    bundle.getString(TimeZoneList.KEY_GMT), // GMT
                    true); // custom
            } else {
                // replaced by on timezone in the list
                changeItem(bundle.getLong(ITEM_POSITION), // position index in the list view
                        bundle.getInt(TimeZoneList.KEY_SEQUENCE, TimeZoneList.SEQUENCE_INVALID),  // sequence
                        "", // city name
                        bundle.getString(TimeZoneList.KEY_TIMEZONE_ID), //  Tiemzone id
                        bundle.getString(TimeZoneList.KEY_GMT), // GMT
                        false); // custom
            }

            break;
        }
    }

    private boolean addTimeZone(int sequence, String city, String timezoneId, String gmt, boolean isCustomTimeZone) {
        if((isCustomTimeZone && (IsStringEmpty(city) || IsStringEmpty(gmt))) ||
                (!isCustomTimeZone && (sequence < 0 || IsStringEmpty(timezoneId)))) {
            // Log.e(TAG, "Got invalid timezone from TimeZoneList");
            return false;
        }
        HashMap<String, Object> map = new HashMap<String, Object>();
        map.put(TimeZoneList.KEY_SEQUENCE, String.valueOf(sequence));
        map.put(TimeZoneList.KEY_CITY, city);
        map.put(TimeZoneList.KEY_TIMEZONE_ID, timezoneId);
        map.put(TimeZoneList.KEY_GMT, gmt);
        map.put(TimeZoneList.KEY_CUSTOM, (isCustomTimeZone ? "TRUE" : "FALSE"));
        mListItem.add(mListItem.size(), map);
        mAdapter.notifyDataSetChanged();

        mDb.openDatabase();
        int _id = -1;
        if(isCustomTimeZone) {
            // _id | city | "" | GMT | custom
            _id = mDb.insert(city, timezoneId, gmt, isCustomTimeZone);
        } else {
            // _id | sequence | timezoneId | GMT | custom
            _id = mDb.insert(String.valueOf(sequence), timezoneId, gmt, isCustomTimeZone);
        }
        //int _id = mDb.insert(String.valueOf(sequence), timezoneId, gmt, false);
        mDb.closeDatabase();
        map.put(_ID, String.valueOf(_id));
        return true;
    }

    private boolean changeItem(long position, int sequence, String city, String timezoneId, String gmt, boolean isCustomTimeZone) {
        if((isCustomTimeZone && (IsStringEmpty(city) || IsStringEmpty(gmt))) ||
                (!isCustomTimeZone && (sequence < 0 || IsStringEmpty(timezoneId)))) {
            // Log.e(TAG, "Got invalid timezone from TimeZoneList");
            return false;
        }
        HashMap<String, Object> map = new HashMap<String, Object>();
        String _id = (String) mListItem.get((int)position).get(_ID);
        map.put(_ID, _id);
        map.put(TimeZoneList.KEY_SEQUENCE, String.valueOf(sequence));
        map.put(TimeZoneList.KEY_CITY, city);
        map.put(TimeZoneList.KEY_TIMEZONE_ID, timezoneId);
        map.put(TimeZoneList.KEY_GMT, gmt);
        map.put(TimeZoneList.KEY_CUSTOM, (isCustomTimeZone ? "TRUE" : "FALSE"));
        mListItem.set((int)position, map);
        mAdapter.notifyDataSetChanged();

        mDb.openDatabase();
        boolean result = false;
        if(isCustomTimeZone) {
            // _id | city | "" (timezoneId) | GMT | custom
            result = mDb.update(_id, city, timezoneId, gmt, isCustomTimeZone);
        } else {
            // _id | sequence | timezoneId | GMT | custom
            result = mDb.update(_id, String.valueOf(sequence), timezoneId, gmt, isCustomTimeZone);
        }
        mDb.closeDatabase();
        return result;
    }

    private void loadFromDatabase(){
        mDb = DatabaseOperator.getInstance(this);
        mDb.openDatabase();
        Cursor cursor = mDb.query(null);
        if(null != cursor) {
            cursor.moveToFirst();
            // Log.d(TAG, "Cursor count: " + cursor.getCount());
            try {
                while(!cursor.isAfterLast()) {
                    HashMap<String, Object> map = new HashMap<String, Object>();
                    map.put(_ID, cursor.getString(DatabaseOperator.INDEX_ID));
                    boolean isCustom = cursor.getString(DatabaseOperator.INDEX_CUSTOM).equals("TRUE");
                    if(isCustom) {
                        map.put(TimeZoneList.KEY_SEQUENCE, "");
                        map.put(TimeZoneList.KEY_CITY, cursor.getString(DatabaseOperator.INDEX_CITY));
                        map.put(TimeZoneList.KEY_TIMEZONE_ID, "");
                        map.put(TimeZoneList.KEY_GMT, cursor.getString(DatabaseOperator.INDEX_GMT));
                        map.put(TimeZoneList.KEY_CUSTOM, "TRUE");
                    } else {
                        map.put(TimeZoneList.KEY_SEQUENCE, cursor.getString(DatabaseOperator.INDEX_SEQUENCE));
                        map.put(TimeZoneList.KEY_CITY, cursor.getString(DatabaseOperator.INDEX_TIMEZONE_ID));
                        map.put(TimeZoneList.KEY_TIMEZONE_ID, cursor.getString(DatabaseOperator.INDEX_TIMEZONE_ID));
                        map.put(TimeZoneList.KEY_GMT, cursor.getString(DatabaseOperator.INDEX_GMT));
                        map.put(TimeZoneList.KEY_CUSTOM, "FALSE");
                    }

                    mListItem.add(mListItem.size(), map);
                    cursor.moveToNext();
                }
            } finally {
                cursor.close();
            }
        }
        mDb.closeDatabase();
    }
    private AlertDialog createDialog(int position) {
        final int pos = position;
        AlertDialog.Builder mAlertDialog = new AlertDialog.Builder(this);
        mAlertDialog.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.delete_timezone_title)
                    .setMessage(R.string.delete_timezone_tip)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                     public void onClick(DialogInterface dialog, int which) {
                         // delete timezone
                         mDb.openDatabase();
                         mDb.delete((String) mListItem.get(pos).get(_ID));
                         mListItem.remove(pos);
                         mAdapter.notifyDataSetChanged();
                         mDb.closeDatabase();
                      }
                    })
                    .setNegativeButton(android.R.string.cancel,
                                 new DialogInterface.OnClickListener() {
                                     public void onClick(DialogInterface dialog, int which) {
                                     }
                    });
        return mAlertDialog.create();
    }

    protected Dialog onCreateDialog(int id) {
        return createDialog(id);
    }

    public boolean onItemLongClick(AdapterView<?> adapter, View item, int position,
            long id) {
        showDialog(position);
        return true;
    }

    public static boolean IsStringEmpty(String str) {
        return null == str || 0 == str.trim().length();
    }
}
