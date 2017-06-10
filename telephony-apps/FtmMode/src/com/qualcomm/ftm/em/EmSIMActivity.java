/******************************************************************************
 * @file    EmSIM1Activity.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.qualcomm.ftm.R;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;


public class EmSIMActivity extends Activity {
    private static final String LOG_TAG = "EmSIMActivity";
    private static final boolean DBG = true;
    private static final int EVENT_DISPLAY_FTM_INFO = 1;

    private ListView mListView = null;
    private Cursor mCursor;
    private DatabaseHelper mDbHelper;
    private SQLiteDatabase mSqliteDatabase;
    private CursorAdapter mCursorAdapter;

    Timer mTimer = new Timer();
    TimerTask mTask = new TimerTask() {
        public void run() {
            Message message = new Message();
            message.what = EVENT_DISPLAY_FTM_INFO;
            handler.sendMessage(message);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.em_list);
        mListView = (ListView) findViewById(R.id.datalist);

        mDbHelper = new DatabaseHelper(EmSIMActivity.this, "em.db");
        mSqliteDatabase = mDbHelper.getReadableDatabase();

        mTimer.schedule(mTask, 0, 1000 * 1);
    }

    final Handler handler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_DISPLAY_FTM_INFO:
                update();
                break;
            }
            super.handleMessage(msg);
        }

        void update() {
            if (mCursor.isClosed()) {
                log("mCursor.isClosed");
                return;
            }

            mCursor.requery();
            mCursorAdapter.notifyDataSetChanged();
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        Intent intent = getIntent();
        int subId = intent.getIntExtra("sub_id", 0);
        log("Current subId :" + subId);
        setupListView(subId);
    }

    @Override
    public void setTitle(CharSequence title) {
        super.setTitle(title);
    }

    private void setupListView(int subId) {
        if (mCursor != null) {
            mCursor.close();
        }
        mCursor = mSqliteDatabase.query("eminfo", new String[]{"_id", "mName", "mValue"},
                "mSub=" + subId, null, null, null, "_id");
        if (mCursor != null) {
            mCursorAdapter = new EmItemListAdapter(EmSIMActivity.this, mCursor);
            mListView.setAdapter(mCursorAdapter);
        }
    }

    private final class EmItemListAdapter extends CursorAdapter {
        Context mContext;
        protected LayoutInflater mInflater;

        public EmItemListAdapter(Context context, Cursor cursor) {
            super(context, cursor, true);

            mContext = context;
            mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View view = mInflater.inflate(R.layout.em_list_item, null);
            return view;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            String name = cursor.getString(cursor.getColumnIndex("mName"));
            long value = cursor.getInt(cursor.getColumnIndex("mValue"));

            ((TextView) view.findViewById(R.id.title)).setText(name);
            ((TextView) view.findViewById(R.id.info)).setText(String.valueOf(value));

            if(name.contains("ncell")) {
                ((TextView) view.findViewById(R.id.info)).setText("");
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mCursor != null) {
            mCursor.close();
        }

        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
    }

    protected void log(String s) {
        android.util.Log.d(LOG_TAG, s);
    }
}
