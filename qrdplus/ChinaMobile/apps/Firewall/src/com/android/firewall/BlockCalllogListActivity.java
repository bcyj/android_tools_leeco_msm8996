/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.firewall;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;
import com.android.firewall.BlockRecord;

public class BlockCalllogListActivity extends Activity{

    public static final String NUMBER_KEY = "number_key";
    private static final int CALL_MENU = 0;
    private static final int MESSAGE_MENU = 1;
    private static final int DELETE_MENU = 2;

     private ListView callBlockRecordListview;
     private TextView callBlockText;
     private TextView emptyCallText;
    private Cursor callCursor;
    private Context mContext;
    private long selectId = -1;
    private String number;
    private void showMenu(View view) {
        PopupMenu popup = new PopupMenu(BlockCalllogListActivity.this, view);
        Menu menu = popup.getMenu();

        menu.add(0, CALL_MENU, 0,
                R.string.block_record_call);
        menu.add(0, MESSAGE_MENU, 0,
                R.string.block_record_message);
        menu.add(0, DELETE_MENU, 0,
                R.string.block_record_delete);
        popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
            public boolean onMenuItemClick(MenuItem item) {
                onContextItemSelected(item);
                return true;
            }
        });
        popup.show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.call_block_list);
        number = getIntent().getStringExtra(NUMBER_KEY);
        mContext = this;
        if (TextUtils.isEmpty(number)) {
            Toast.makeText(this, R.string.number_is_empty, Toast.LENGTH_LONG).show();
            finish();
        }
        init();
    }

     public void init() {

            callBlockRecordListview = (ListView)findViewById(R.id.callBlockRecordListview);

            emptyCallText = (TextView) findViewById(R.id.call_record_is_empty);

            initListView();
        }

    private void initListView() {
        if(number.startsWith("+") || number.startsWith("00")){
            int len = number.length();
            if (len > 11) {
                number = number.substring(len - 11, len);
            }
        }

        callCursor = getContentResolver().query(
                BlockRecord.CONTENT_URI, new String[]{
                    "_id","block_type","contact","date","data",
                },
                " block_type = "+"0" + " AND " + "contact" + " LIKE '%" + number + "'",null,
               null);
        callBlockRecordListview.setEmptyView(emptyCallText);

        CallBlockListCursorAdapter callAdapter = new CallBlockListCursorAdapter(
                this, callCursor, true);

        callBlockRecordListview.setAdapter(callAdapter);

        final List<String> menuList = Arrays
                .asList(BlockCalllogListActivity.this.getResources().getStringArray(
                        R.array.block_record_menu_array));
        callBlockRecordListview
                .setOnItemClickListener(new OnItemClickListener() {

                    @Override
                    public void onItemClick(AdapterView<?> arg0, View view, int arg2, long arg3) {
                        callCursor.moveToPosition(arg2);
                        selectId = callCursor.getLong(callCursor
                                .getColumnIndexOrThrow(BlockRecord._ID));
                        showMenu(view);
                    }
                });
    }

    class CallBlockListCursorAdapter extends CursorAdapter {

        private LayoutInflater mInflater;
        private int indexOfNumber, indexOfTime;
        private Handler handler;
        private int position;

        public CallBlockListCursorAdapter(Context context, Cursor c,
                boolean autoRequery) {
            super(context, c, autoRequery);

            mInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            getColumnIndices(c);
            c.setNotificationUri(getContentResolver(),
                    BlockRecord.CONTENT_URI);
            handler = new Handler();
            position = 0;
        }

        private void getColumnIndices(Cursor cursor) {
            if (cursor != null) {
                indexOfNumber = cursor.getColumnIndexOrThrow(BlockRecord.CONTACT);
                indexOfTime = cursor.getColumnIndexOrThrow(BlockRecord.DATE);
            }
        }

        @Override
        public int getCount() {

            if (callCursor.getCount() != 0
                    && callBlockRecordListview.getVisibility() == View.GONE) {
                callBlockRecordListview.setVisibility(View.VISIBLE);
                emptyCallText.setVisibility(View.GONE);

            } else if (callBlockRecordListview.getVisibility() == View.VISIBLE
                    && callCursor.getCount() == 0) {
                callBlockRecordListview.setVisibility(View.GONE);
                emptyCallText.setVisibility(View.VISIBLE);
            }
            return super.getCount();
        }

        @Override
        public void bindView(View arg0, Context arg1, Cursor arg2) {

            if (callCursor.getCount() != 0
                    && callBlockRecordListview.getVisibility() == View.GONE) {
                callBlockRecordListview.setVisibility(View.VISIBLE);
                emptyCallText.setVisibility(View.GONE);
            }
            position++;
            TextView time = (TextView) arg0.findViewById(R.id.time);

            String timeStr = arg2
                    .getString(indexOfTime);

            if (!TextUtils.isEmpty(timeStr)) {
                time.setText(getTimeStr(Long.parseLong(timeStr)));
            }

        }

        @Override
        public View newView(Context arg0, Cursor arg1, ViewGroup arg2) {

            return mInflater
                    .inflate(R.layout.call_block_list_item, arg2, false);
        }
    }

    private String getTimeStr(long time) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return format.format(new Date(time));
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case CALL_MENU: {
                Intent intent = new Intent(Intent.ACTION_CALL, Uri.parse("tel:" + number));
                startActivity(intent);
                break;
            }
            case MESSAGE_MENU: {
                Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse("smsto:" + number));
                startActivity(intent);
                return true;
            }
            case DELETE_MENU:
                getContentResolver().delete(BlockRecord.CONTENT_URI, " _id = ?  ", new String[] {
                        String.valueOf(selectId)
                });
                return true;
        }
        return false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (callCursor != null && !callCursor.isClosed()) {
            callCursor.close();
        }
    }
}
