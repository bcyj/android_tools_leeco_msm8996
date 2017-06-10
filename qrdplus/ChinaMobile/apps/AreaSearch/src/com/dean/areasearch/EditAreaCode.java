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
package com.dean.areasearch;

import android.app.Activity;
import android.content.ContentUris;
import android.content.ContentValues;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface;
import android.content.ContentResolver;
import android.database.Cursor;
import android.text.InputFilter;
import android.util.Log;
import com.dean.areasearch.AreaCodeTables.AreaCodeTable;

public class EditAreaCode extends Activity implements RadioGroup.OnCheckedChangeListener,
View.OnClickListener
{
    private static String TAG = "EditAreaCode";

    private long mId;
    private boolean mbEdit;
    private String mCode;
    private String mCityName;
    private RadioGroup mRadioGroup;
    private String mExistCode;
    private String mExistName;
    private long mExistId;
    String areaName;
    TextView selecttext ;
    EditText etext;
    EditText telnumber;
    int maxlength = 4;

    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setTitle(R.string.str_menu_add);
        setContentView(R.layout.edit_area_code);

        mRadioGroup = (RadioGroup) findViewById(R.id.menu);

        selecttext = (TextView)findViewById(R.id.selecttext);

        mRadioGroup.setOnCheckedChangeListener(this);
        etext = (EditText)findViewById(R.id.EditText_cityname);
        Bundle bundle = getIntent().getExtras();
        if (bundle != null)
        {
           // setTitle(R.string.str_title_edit);
            mbEdit = true;

            mId = bundle.getLong("id");
            mCode = bundle.getString("code");
            mCityName = bundle.getString("cityName");

            ((EditText)findViewById(R.id.EditText_code)).setText(mCode);
            ((EditText)findViewById(R.id.EditText_cityname)).setText(mCityName);
            ((Button)findViewById(R.id.Button_delete)).setVisibility(View.VISIBLE);
        }
        else
        {
          //  setTitle(R.string.str_title_new);
            mbEdit = false;
            ((Button)findViewById(R.id.Button_delete)).setVisibility(View.GONE);
        }

        /* save button, save data to database */
        telnumber = (EditText)findViewById(R.id.EditText_code);

        ((Button)findViewById(R.id.Button_save)).setOnClickListener(new View.OnClickListener(){
            public void onClick(View view)
            {
                int buttonid=mRadioGroup.getCheckedRadioButtonId();
                etext = (EditText)findViewById(R.id.EditText_cityname);
                areaName = etext.getText().toString();
                Log.w(TAG, areaName);
                if(areaName.length() == 0)
                {
                    showMessageBox(EditAreaCode.this, R.string.msg_area_null);
                    return;
                }


                String telNum = telnumber.getText().toString();
                int len = telNum.length();
             if (len == 0)
                {
                    showMessageBox(EditAreaCode.this, R.string.msg_numb_null);
                    return;
                }
             else if (!isDigitStr(telNum))
                {
                    showMessageBox(EditAreaCode.this, R.string.wrong_include_non_dig);
                    return;
                }
            else if (buttonid == R.id.domestic)
                {
                 if(telNum.charAt(0)!='0')
                {
                    showMessageBox(EditAreaCode.this, R.string.domestic_0);
                    return;
                }
                 if(len<3||len>4)
                 {
                     showMessageBox(EditAreaCode.this, R.string.domestic_34);
                     return;
                 }
                }
             else if (buttonid == R.id.mobile)
                {
                 if(telNum.charAt(0)!='1')
                {
                    showMessageBox(EditAreaCode.this, R.string.mobile_1);
                    return;
                }
                 if(len!=8)
                 {
                     showMessageBox(EditAreaCode.this, R.string.mobile_7);
                     return;
                 }
                }
            else if (buttonid ==R.id.international)
                {

                if(len ==1)
                {
                    showMessageBox(EditAreaCode.this, R.string.international_00);
                    return;
                }
                else if((telNum.charAt(0)!='0')||(telNum.charAt(1)!='0'))
                {
                    showMessageBox(EditAreaCode.this, R.string.international_00);
                    return;
                }

                }
                final ContentValues values = new ContentValues();
                values.put(AreaCodeTable.CODE, telNum);
                values.put(AreaCodeTable.CITY_NAME, areaName);

                if(mbEdit)
                {
                    // edit data
                    if(!telNum.equals(mCode) || !areaName.equals(mCityName))
                    {
                        boolean exist = queryByCode(telNum);
                        if (!exist || (exist && mExistId == mId))
                        {
                            // save updated data

                            Uri uri = ContentUris.withAppendedId(AreaCodeTable.CONTENT_URI, mId);
                            getContentResolver().update(uri, values, null, null);
                        }
                        else
                        {
                            String msg = getString(R.string.msg_exist_similar_code2,
                                mExistCode + " : " + mExistName);
                            new AlertDialog.Builder(EditAreaCode.this)
                            .setIconAttribute(android.R.attr.alertDialogIcon)
                            .setTitle(R.string.message_box_title)
                            .setMessage(msg)
                            .setPositiveButton(android.R.string.ok,null)
                            .show();
                            return;
                        }
                    }
                }
                else
                {
                    // add new data
                    boolean exist = queryByCode(telNum);
                    if (!exist)
                    {
                        // save newly added data
                        getContentResolver().insert(AreaCodeTable.CONTENT_URI, values);
                    }
                    else
                    {
                        String msg = getString(R.string.msg_exist_similar_code1, mExistCode + " : " + mExistName);
                        showMessageBox(EditAreaCode.this, msg, new OnClickListener() {
                            public void onClick(DialogInterface dialog, int which)
                            {
                                Uri uri = ContentUris.withAppendedId(AreaCodeTable.CONTENT_URI, mExistId);
                                getContentResolver().update(uri, values, null, null);
                                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                                imm.hideSoftInputFromWindow(etext.getWindowToken(), 0);
                                finish();
                            }
                        });
                        return;
                    }
                }
                // close keyboard
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(etext.getWindowToken(), 0);
                finish();
            } // end of onClick() of save button
        });

        telnumber.setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxlength)});

        /* cancel button , abort edit */
        ((Button)findViewById(R.id.Button_cancel)).setOnClickListener(new View.OnClickListener() {
            public void onClick(View view)
            {
                // close keyboard
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(etext.getWindowToken(), 0);
                finish();
            }
        });

        /* delete button */
        ((Button)findViewById(R.id.Button_delete)).setOnClickListener(new View.OnClickListener() {
            public void onClick(View view)
            {
                final ContentResolver contentResolver = getContentResolver();
                new AlertDialog.Builder(EditAreaCode.this)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.message_box_title)
                .setMessage(R.string.delete_query)
                .setNegativeButton(R.string.button_cancel, null)
                .setPositiveButton(R.string.button_ok, new OnClickListener(){
                    public void onClick(DialogInterface dialog, int which)
                    {
                        // delete selected data
                        Uri uri = ContentUris.withAppendedId(AreaCodeTable.CONTENT_URI, mId);
                        contentResolver.delete(uri, null, null);
                        finish();
                    }})
                .show();
            }
        });
    }

    public static void showMessageBox(Context context, int msgId)
    {
        new AlertDialog.Builder(context)
        .setIconAttribute(android.R.attr.alertDialogIcon)
        .setTitle(R.string.message_box_title)
        .setMessage(msgId)
        .setPositiveButton(android.R.string.ok,null)
        .show();
    }

    public static void showMessageBox(Context context, String msg, OnClickListener callback)
    {
        new AlertDialog.Builder(context)
        .setIconAttribute(android.R.attr.alertDialogIcon)
        .setTitle(R.string.message_box_title)
        .setMessage(msg)
        .setNegativeButton(android.R.string.cancel, null)
        .setPositiveButton(android.R.string.ok, callback)
        .show();
    }

    private boolean isDigitStr(String input)
    {
        int len = input.length();
        for(int i=0; i<len; i++)
        {
            char c = input.charAt(i);
            if(c > '9' || c < '0')
                return false;
        }
        return true;
    }

    // @param code : area code
    private boolean queryByCode(String code)
    {
        String queryStr = AreaCodeTable.CODE + "='" + code + "'";
        Cursor cursor = getContentResolver().query(AreaCodeTable.CONTENT_URI,
                new String[]{"_id", "cityname"}, queryStr, null, null);
        if(cursor != null && cursor.getCount() > 0)
        {
            cursor.moveToFirst();
            mExistId = cursor.getLong(0);
            mExistName = cursor.getString(1);
            mExistCode = code;
            cursor.close();
            return true;
        }
        return false;
    }


    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
      int     buttonid = mRadioGroup.getCheckedRadioButtonId();
        if (buttonid == R.id.domestic)
        {
        selecttext.setText(R.string.domestic_selected);
        maxlength = 4;
        telnumber.getEditableText().clear();
        telnumber.setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxlength)});

        }
        if (buttonid == R.id.mobile)
        {
        maxlength = 8;
        telnumber.getEditableText().clear();
        telnumber.setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxlength)});
        selecttext.setText(R.string.mobile_selected);
        }
        if (buttonid == R.id.international)
        {
        maxlength = 6;
        telnumber.getEditableText().clear();
        telnumber.setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxlength)});
        selecttext.setText(R.string.international_selected);
        }
    }

}
