/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

import java.io.IOException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.TimeZone;

import org.xmlpull.v1.XmlPullParserException;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
// import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

public class TimeZoneList extends ListActivity{
//    private final static String TAG = "TimeZoneList";
    private final static String TAG_TIMEZONE = "timezone";
    private final static String TAG_ID = "id";

    final static String KEY_SEQUENCE = "sequence";
    final static String KEY_TIMEZONE_ID = "timezone_id";
    final static String KEY_CITY = "city";
    final static String KEY_GMT="GMT";
    final static String KEY_CUSTOM = "custom";
    final static String KEY_DISPLAY_NAME = "name";
    final static int SEQUENCE_INVALID = -1;
    final static int SEQUENCE_CUSTOMER = -2;

    final static String GMT_POSITIVE = "GMT+";
    final static String GMT_NEGATIVE = "GMT-";

    private final static int MILLISECOND_PER_HOUR = 3600000;
    private final static int MILLISECOND_PER_MINUTE = 60000;

    private final static int FETCH_TIMEZONE_START_TAG = 1;
    private final static int FETCH_TIMEZONE_DISPLAY_NAME = 3;
    private final static int FETCH_TIMEZONE_END_TAG = 4;

    private final static int MODE_UNKNOWN = -1;
    private final static int MODE_ADD_TIME_ZONE = 0;
    private final static int MODE_SELECT_TIME_ZONE = 1;

    private final static int MAX_TIME_ZONE_HOUR = 14;
    private final static int MAX_TIME_ZONE_MINUTE = 59;

    private int mSequence = SEQUENCE_INVALID;
    private String mTimezoneId = null;
    private List<HashMap<String, Object>> mTimezoneSortedList = null;
    private int mMode = MODE_UNKNOWN;
    private long mPosition = -1;

    private Button mBtnSign = null;
    private EditText mEtCity = null;
    private EditText mEtHour = null;
    private EditText mEtMinute = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle bundle = getIntent().getExtras();
        String action = bundle.getString(QWorldClock.ACTION);
        mPosition = -1;
        if(action.equals(QWorldClock.ACTION_ADD_TIME_ZONE)) {
            mMode = MODE_ADD_TIME_ZONE;
        } else if(action.equals(QWorldClock.ACTION_SELECT_TIME_ZONE)) {
            mMode = MODE_SELECT_TIME_ZONE;
            mPosition = bundle.getLong(QWorldClock.ITEM_POSITION);
        } else {
            mMode = MODE_UNKNOWN;
        }

        getListView().addHeaderView(buildHeader());
        mTimezoneSortedList = getZoneList();
        SimpleAdapter adapter = new SimpleAdapter(this,
                (List)mTimezoneSortedList,
                android.R.layout.simple_list_item_2,
                new String[] {KEY_DISPLAY_NAME, KEY_GMT},
                new int[] {android.R.id.text1, android.R.id.text2});
        setListAdapter(adapter);
        setResult(RESULT_CANCELED); // send result code RESULT_CANCELED when user press back key in this activity
    }

    private View buildHeader() {
        Button btn = new Button(this);
        btn.setText(getResources().getString(R.string.add_custom_timezone));
        btn.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                AlertDialog.Builder builder =
                    new AlertDialog.Builder(TimeZoneList.this)
                            .setTitle(R.string.custom_timezone_header)
                            .setOnCancelListener(new OnCancelListener() {
                                public void onCancel(DialogInterface dialog) {
                                    finish();
                                }
                            })
                            .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    boolean cityEmpty = false;
                                    if(mEtCity.getText().toString().trim().length() == 0) {
                                        cityEmpty = true;
                                        Toast.makeText(TimeZoneList.this, R.string.city_field_empty_prompt, Toast.LENGTH_SHORT).show();
                                    }

                                    //check hour and minute fields should not be empty
                                    boolean bHourEmpty = mEtHour.getText().toString().isEmpty();
                                    boolean bMinuteEmpty = mEtMinute.getText().toString().isEmpty();
                                    if(!cityEmpty && bHourEmpty){
                                        Toast.makeText(TimeZoneList.this,R.string.hour_field_empty_prompt,Toast.LENGTH_SHORT).show();
                                    }
                                    if(!cityEmpty && !bHourEmpty && bMinuteEmpty){
                                        Toast.makeText(TimeZoneList.this,R.string.minute_field_empty_prompt,Toast.LENGTH_SHORT).show();
                                    }
                                    // If the city field is empty, force the dialog dismissing
                                    Field field;
                                    try {
                                        field = dialog.getClass().getSuperclass().getDeclaredField("mShowing");
                                        field.setAccessible(true);
                                        field.set(dialog, !cityEmpty && !bHourEmpty && !bMinuteEmpty);
                                    } catch (Exception e) {
                                        e.printStackTrace();
                                    }

                                    if(!cityEmpty && !bHourEmpty && !bMinuteEmpty) {
                                        Bundle bundle = new Bundle();
                                        bundle.putString(KEY_CITY, mEtCity.getText().toString());
                                        bundle.putString(KEY_GMT, getGmtFromDialog());
                                        bundle.putBoolean(KEY_CUSTOM, true);
                                        Intent intent = new Intent();
                                        if(MODE_ADD_TIME_ZONE == mMode) {      // add timezone
                                            intent.putExtras(bundle);
                                            setResult(QWorldClock.RESULT_ADD_TIME_ZONE, intent);
                                        } else if (MODE_SELECT_TIME_ZONE == mMode){ // change timezone
                                            bundle.putLong(QWorldClock.ITEM_POSITION, mPosition);
                                            intent.putExtras(bundle);
                                            setResult(QWorldClock.RESULT_SELECT_TIME_ZONE, intent);
                                        }
                                        finish();
                                    }
                                }
                            })
                            .setNegativeButton(android.R.string.cancel,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog, int which) {
                                            boolean cityEmpty = false;
                                            if (mEtCity.getText().toString().trim().length() == 0) {
                                               cityEmpty = true;
                                            }
                                            boolean bHourEmpty = mEtHour.getText().toString()
                                                    .isEmpty();
                                            boolean bMinuteEmpty = mEtMinute.getText()
                                                    .toString().isEmpty();
                                            Field field;
                                            try {
                                                field = dialog.getClass().getSuperclass()
                                                        .getDeclaredField("mShowing");
                                                field.setAccessible(true);
                                                if (cityEmpty || bHourEmpty || bMinuteEmpty) {
                                                   field.set(dialog, true);
                                                }

                                            } catch (Exception e) {
                                                e.printStackTrace();
                                            }
                                        }
                                    });              
                
                AlertDialog dialog = builder.create();
                dialog.setView(buildCustomEditor(), 0, 0, 0, 0);
                dialog.show();
            }
        });
        return btn;
    }

    private String getGmtFromDialog() {
        StringBuilder sb = new StringBuilder();
        if(mBtnSign.getText().equals(getResources().getString(R.string.sign_pos))) {
            sb.append(GMT_POSITIVE);
        } else {
            sb.append(GMT_NEGATIVE);
        }
        sb.append(Integer.valueOf(mEtHour.getText().toString())).append(':');
        Integer minute = Integer.valueOf(mEtMinute.getText().toString());
        if(minute < 10) {
            sb.append('0');
        }
        sb.append(minute);
        return sb.toString();
    }

    private View buildCustomEditor() {
        LayoutInflater inflater = LayoutInflater.from(this);
        View custom = inflater.inflate(R.layout.custom_time_zone, null);

        mBtnSign = (Button) custom.findViewById(R.id.btn_gmt_sign);
        mBtnSign.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                // Reverse the sign each time the button was pressed
                Button btn = (Button)v;
                String sign = (String) btn.getText();
                if(sign.equals(getResources().getString(R.string.sign_pos))){
                    btn.setText(getResources().getString(R.string.sign_neg));
                } else {
                    btn.setText(getResources().getString(R.string.sign_pos));
                }
            }
        });

        OnFocusChangeListener focusListener = new OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                // Abandon the prefix 0 when the edittext lose focus
                if (!hasFocus) {
                    String strValue = ((EditText) v).getText().toString();
                    String strResult = null;
                    if(strValue.trim().equals("")) {
                        strResult = "0";
                    } else {
                        strResult = Integer.valueOf(strValue).toString();
                    }
                    ((EditText) v).setText(strResult);
                }
            }
        };

        mEtHour = (EditText) custom.findViewById(R.id.et_gmt_hour);
        InputFilter hourFilter = new InputFilter() {
            public CharSequence filter(CharSequence source, int start, int end,
                    Spanned dest, int dstart, int dend) {
                CharSequence filtered = source.subSequence(start, end);
                if(filtered.equals("")) { // It means that backspace key was pressed
                    return "";
                }
                for(int i = 0; i < filtered.length(); ++i) {
                    if(filtered.charAt(i) > '9' || filtered.charAt(i) < '0') {
                        return "";
                    }
                }

                String combine = String.valueOf(dest.subSequence(0, dstart)) + filtered + dest.subSequence(dend, dest.length());
                // If the combine is "", it should not to be use to Integer.valueOf, because it is not a number.
                if ((combine.equals("") || combine.length() >= 3) || (Integer.valueOf(combine) > MAX_TIME_ZONE_HOUR)) {
                    return ""; // return "" means nothing new was added
                }
                return null; // return null means that input what the user typed
            }
        };
        mEtHour.setFilters(new InputFilter[] {hourFilter});
        mEtHour.setOnFocusChangeListener(focusListener);

        mEtMinute = (EditText) custom.findViewById(R.id.et_gmt_minute);
        InputFilter minuteFilter = new InputFilter() {
            public CharSequence filter(CharSequence source, int start, int end,
                    Spanned dest, int dstart, int dend) {
                CharSequence filtered = source.subSequence(start, end);
                if(filtered.equals("")) { // It means that backspace key was pressed
                    return "";
                }

                for(int i = 0; i < filtered.length(); ++i) {
                    if(filtered.charAt(i) > '9' || filtered.charAt(i) < '0') {
                        return "";
                    }
                }

                String combine = String.valueOf(dest.subSequence(0, dstart)) + filtered + dest.subSequence(dend, dest.length());
                // There is a possible that the combine is "", Integer.valueOf a non number will crash. add condition to avoid
                // this happen.
                if((combine.equals("")) || (combine.length() >= 3) || (Integer.valueOf(combine) > MAX_TIME_ZONE_MINUTE)) {
                    return ""; // return "" means nothing new was added
                }
                return null; // return null means that input what the user typed
            }
        };
        mEtMinute.setFilters(new InputFilter[] {minuteFilter});
        mEtMinute.setOnFocusChangeListener(focusListener);

        mEtCity = (EditText) custom.findViewById(R.id.et_city);
        mEtCity.addTextChangedListener(tw);
        return custom;
    }

    private TextWatcher tw = new TextWatcher(){
        CharSequence previousString;
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count,
                int after) {
            previousString = s.toString();
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before,
                int count) {
                if (containSpecialChar(s.toString())) {
                    showAlert();
                    mEtCity.setText(previousString);
                    mEtCity.setSelection(mEtCity.getText().toString().length());
                }
            }

        @Override
        public void afterTextChanged(Editable s) {
        }
    };

    private boolean containSpecialChar(String name){
        CharSequence[] specialChar = new CharSequence[]{",", ".", "@", "#", "%",
                "&", "*", "(", ")", "-", "+", "\"", ":", "?", "$", "!", "\'", ";", "/"};

        for(int i = 0; i < specialChar.length; i++){
            if (name.contains(specialChar[i])) {
                return true;
            }
        }
        return false;
    }

    private void showAlert(){
        Toast.makeText(getApplicationContext(), R.string.illegal_city_name, Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Intent intent = new Intent();
        Bundle bundle = new Bundle();
        HashMap<String, Object> map = (HashMap<String, Object>) l.getItemAtPosition(position);
        if (map == null) return;

        switch(mMode) {
        case MODE_ADD_TIME_ZONE:
            bundle.putInt(KEY_SEQUENCE, (Integer)map.get(KEY_SEQUENCE));
            bundle.putString(KEY_TIMEZONE_ID, (String)map.get(KEY_TIMEZONE_ID));
            bundle.putString(KEY_GMT, (String)map.get(KEY_GMT));
            intent.putExtras(bundle);
            setResult(QWorldClock.RESULT_ADD_TIME_ZONE, intent);
            break;
        case MODE_SELECT_TIME_ZONE:
            bundle.putInt(KEY_SEQUENCE, (Integer)map.get(KEY_SEQUENCE));
            bundle.putString(KEY_TIMEZONE_ID, (String)map.get(KEY_TIMEZONE_ID));
            bundle.putString(KEY_GMT, (String)map.get(KEY_GMT));
            bundle.putLong(QWorldClock.ITEM_POSITION, mPosition);
            intent.putExtras(bundle);
            setResult(QWorldClock.RESULT_SELECT_TIME_ZONE, intent);
            break;
        }
        finish();
    }

    private List<HashMap<String, Object>> getZoneList() {
        List<HashMap<String, Object>> zoneList = new ArrayList<HashMap<String, Object>>();
        try {
            XmlResourceParser parser = getResources().getXml(R.xml.timezones);
            boolean stop = false;
            int fetchStatus = FETCH_TIMEZONE_START_TAG;
            int eventType = parser.getEventType();
            while(!stop) {
                switch(eventType) {
                case XmlResourceParser.START_TAG:
                    if(FETCH_TIMEZONE_START_TAG != fetchStatus) handleParseError(parser);
                    fetchStatus = parseStartTag(parser, fetchStatus);
                    break;
                case XmlResourceParser.END_TAG:
                    // may got the outer layer's END_TAG
                    if(FETCH_TIMEZONE_END_TAG != fetchStatus && FETCH_TIMEZONE_START_TAG != fetchStatus) handleParseError(parser);
                    if(parser.getName().equals(TAG_TIMEZONE) && mSequence >= 0 && null != mTimezoneId) {
                        fetchStatus = FETCH_TIMEZONE_START_TAG;
                        addTimeZone(zoneList, mSequence, mTimezoneId);
                        mSequence = SEQUENCE_INVALID;
                        mTimezoneId = null;
                    }
                    break;
                case XmlResourceParser.TEXT:
                    if(FETCH_TIMEZONE_DISPLAY_NAME != fetchStatus) handleParseError(parser);
                    mTimezoneId = parser.getText();
                    fetchStatus = FETCH_TIMEZONE_END_TAG;
                    break;
                case XmlResourceParser.START_DOCUMENT:
                    if(FETCH_TIMEZONE_START_TAG != fetchStatus) handleParseError(parser);
                    break;
                case XmlResourceParser.END_DOCUMENT:
                    if(FETCH_TIMEZONE_START_TAG != fetchStatus) handleParseError(parser);
                    stop = true;   // reach the end of the document, break the loop
                    break;
                case XmlResourceParser.COMMENT:
                    // Log.d(TAG, "COMMENT");
                    break;
                default:
                    break;
                }

                eventType = parser.nextToken();
            }
            parser.close();

        } catch(XmlPullParserException xe) {
            // Log.e(TAG, "xml parse exception");
        } catch(IOException ioe) {
            // Log.e(TAG, "io exception when reading xml");
        }
        return zoneList;
    }

    private void addTimeZone(List<HashMap<String, Object>> zoneList, int sequence, String timezoneId) {
        HashMap<String, Object> map = new HashMap<String, Object>();
        map.put(KEY_SEQUENCE, sequence);
        map.put(KEY_TIMEZONE_ID, timezoneId);
        map.put(KEY_GMT, getGmtFromId(timezoneId));
        map.put(KEY_DISPLAY_NAME, (getResources().getStringArray(R.array.city_names))[sequence - 1]);
        zoneList.add(map);
    }

    private String getGmtFromId(String id) {
        StringBuilder sb = new StringBuilder();
        long reference = Calendar.getInstance().getTimeInMillis();
        int offset = TimeZone.getTimeZone(id).getOffset(reference);
        if(offset < 0) {
            sb.append(GMT_NEGATIVE);
            offset *= -1;
        } else {
            sb.append(GMT_POSITIVE);
        }
        sb.append(offset / MILLISECOND_PER_HOUR).append(':');
        int minute = (offset % MILLISECOND_PER_HOUR) / MILLISECOND_PER_MINUTE;
        if(minute < 10) {
            sb.append('0');
        }
        sb.append(minute);
        return sb.toString();
    }

    private void handleParseError(XmlResourceParser parser) throws XmlPullParserException {
        parser.close();
        // Log.e(TAG, "document format error!");
        throw new XmlPullParserException("document format error!");
    }

    private int parseStartTag(XmlResourceParser parser, int status) {
        if(parser.getName().equals(TAG_TIMEZONE) &&
            parser.getAttributeName(0).equals(TAG_ID) && FETCH_TIMEZONE_START_TAG == status) {
            // Got "timezone" start tag and "id" tag
            status = FETCH_TIMEZONE_DISPLAY_NAME;
            try {
                mSequence = Integer.parseInt(parser.getAttributeValue(0));
            } catch (NumberFormatException e) {
                mSequence = SEQUENCE_INVALID;   // indicates an invalid sequence
            }
        }
        return status;
    }
}
