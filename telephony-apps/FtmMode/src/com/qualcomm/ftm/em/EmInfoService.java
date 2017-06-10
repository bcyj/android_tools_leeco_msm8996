/******************************************************************************
 * @file    EmInfoService.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;


import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.qualcomm.ftm.em.model.OverlayService;
import com.qualcomm.ftm.R;

import java.util.Timer;
import java.util.TimerTask;

public class EmInfoService extends OverlayService {

    private DatabaseHelper mDbHelper;
    private SQLiteDatabase mSqliteDatabase;
    private CursorAdapter mCursorAdapter;
    private Timer mTimer = null;
    private TimerTask mTask = null;
    private int mNumPhones = TelephonyManager.getDefault().getPhoneCount();
    private ListView[] mSubListview = new ListView[mNumPhones];
    private Cursor[] mCursor = new Cursor[mNumPhones];
    private TextView[] mSubTextView =  new TextView[mNumPhones];
    private int[] mResourceListview = {R.id.sub1_listview, R.id.sub2_listview,
            R.id.sub3_listview};
    private int[] mResourceString = {R.string.sub1, R.string.sub2, R.string.sub3};
    private int[] mResourceSubs = {R.id.sub1, R.id.sub2, R.id.sub3};

    private static final int EVENT_DISPLAY_FTM_INFO = 1;

    protected WindowManager.LayoutParams setLayoutParams(){
        mParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.type = WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY;
        mParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        mParams.format = PixelFormat.TRANSLUCENT;
        // set Gravity of the display
        mParams.gravity = Gravity.LEFT | Gravity.TOP;
        return mParams;
    }

    final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_DISPLAY_FTM_INFO:
                update();
                break;
            }
            super.handleMessage(msg);
        }

        void update() {
            setupListView();
        }
    };

    @Override
    protected View setView() {
        return sLayoutInflater.inflate(R.layout.datalist, null);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mDbHelper = new DatabaseHelper(this,"em.db");
        if (mSqliteDatabase != null) {
            mSqliteDatabase.close();
        }
        mSqliteDatabase = mDbHelper.getReadableDatabase();
        for (int i = 0; i < mNumPhones; i++) {
            mSubListview[i] = (ListView)mView.findViewById(mResourceListview[i]);
            mSubTextView[i] = (TextView)mView.findViewById(mResourceSubs[i]);
        }
        setupListView();

        boolean startOverlay = intent.getExtras().getBoolean("startOverlay");
        if (!startOverlay && mTimer!=null) {
            mTimer.cancel();
            mTimer = null;
        } else {
            mTimer = new Timer();
            mTask = new TimerTask() {
                public void run() {
                    Message message = new Message();
                    message.what = EVENT_DISPLAY_FTM_INFO;
                    mHandler.sendMessage(message);
                }
            };
            mTimer.schedule(mTask, 0, 1000 * 1);
        }

            return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
        for (int i = 0; i < mNumPhones; i++) {
            if (mCursor[i] != null) {
                mCursor[i].close();
            }
        }
        if (mSqliteDatabase != null) {
            mSqliteDatabase.close();
        }
    }

    private void setupListView() {
        for (int i = 0; i < mNumPhones; i++) {
            mSubTextView[i].setText(getResources().getString((mNumPhones == 1) ?
                    R.string.singlesim : mResourceString[i]));
            if (mCursor[i] != null) {
                mCursor[i].close();
            }
            mCursor[i] = mSqliteDatabase.query("eminfo", new String[]{"_id", "mName",
                    "mValue"}, "mSub=" + i + " and mIsNotImportant<>1", null, null, null, "_id");
            if (mCursor[i] != null) {
                mCursorAdapter = new EmItemListAdapter(this, mCursor[i]);
                mSubListview[i].setAdapter(mCursorAdapter);
            }
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
            View view = mInflater.inflate(R.layout.data_list_item, null);
            return view;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            String name = cursor.getString(cursor.getColumnIndex("mName"));
            long value = cursor.getInt(cursor.getColumnIndex("mValue"));

            ((TextView) view.findViewById(R.id.title)).setText(name + ":  ");
            ((TextView) view.findViewById(R.id.info)).setText(String.valueOf(value));
        }
    }
}
