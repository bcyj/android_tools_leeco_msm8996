/******************************************************************************
 * @file    EmBroadcastReceiver.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;

import com.qualcomm.ftm.em.Rat;
import com.qualcomm.qcrilhook.PrimitiveParser;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

public class EmBroadcastReceiver extends BroadcastReceiver {

    private static final String LOG_TAG = "EmBroadcastReceiver";
    private static final boolean DBG = true;
    private static final int SECRET_CODE = 3878;
    private static final String SECRET_CODE_ACTION =
            "android.provider.Telephony.SECRET_CODE";
    private static final String EM_CODE_ACTION =
            "qualcomm.intent.action.ACTION_EM_DATA_RECEIVED";
    private static final int MAX_CELLS = 6;

    private SQLiteDatabase mSqliteDatabase;
    private ArrayList<EmInfo> mList;
    private Context mContext;
    private ByteBuffer mRespByteBuf;

    public void onReceive(Context context, Intent intent) {
        mContext = context;

        if (intent.getAction().equals(SECRET_CODE_ACTION)) {
            String host = intent.getData() != null ? intent.getData().getHost() : null;
            if (null != host){
                int code;
                try {
                    code = Integer.parseInt(host);
                    switch (code){
                    case SECRET_CODE:
                        if (DBG) log("start EngineerMode");
                        Intent newIntent = new Intent(Intent.ACTION_MAIN);
                        newIntent.setClass(context, EngineerModeActivity.class);
                        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        context.startActivity(newIntent);
                        break;
                    default :
                        break;
                    }
                } catch (NumberFormatException e) {
                    log("Number Format Exception parsing secret code" + e);
                }
            } else {
               log("host is null");
            }
        } else if (intent.getAction().equals(EM_CODE_ACTION)) {
            DatabaseHelper mDbHelper = new DatabaseHelper(mContext, "em.db");
            mSqliteDatabase = mDbHelper.getReadableDatabase();
            Bundle bundle = intent.getExtras();
            byte buffer[] = bundle.getByteArray("em_data");
            int sub = bundle.getInt("sub_id");

            if (DBG) {
                log("buffer.length is:" + buffer.length);
                for (int i = 0; i < buffer.length; i++) {
                    log("buffer["+i+"]:" + buffer[i]);
                }
            }
            mRespByteBuf = ByteBuffer.wrap(buffer);
            mRespByteBuf.order(ByteOrder.nativeOrder());
            mList = new ArrayList<EmInfo>();
            log("ratType is " + EngineerModeActivity.ratType[sub] + "sub is" + sub);
            setCommonBaseInfo(sub);
            if (EngineerModeActivity.ratType[sub] == Rat.CDMA) {
                setCdmaInfo(sub);
            } else if (EngineerModeActivity.ratType[sub] == Rat.GSM) {
                setGsmInfo(sub);
            } else if (EngineerModeActivity.ratType[sub] == Rat.WCDMA) {
                setWcdmaInfo(sub);
            }
            insertEmInfo(sub);
            mSqliteDatabase.close();
        }
    }


    private void setWcdmaNCellInfo(int sub) {
        long cellId = PrimitiveParser.toUnsigned(mRespByteBuf.getInt());
        mList.add(new EmInfo("wncell " + cellId + ":", cellId, sub, 1));
        mList.add(new EmInfo("dl_uarfcn", PrimitiveParser.toUnsigned(
                mRespByteBuf.getShort()), sub, 1));
        mList.add(new EmInfo("psc", PrimitiveParser.toUnsigned(
                mRespByteBuf.getShort()), sub, 1));
        mList.add(new EmInfo("rscp", mRespByteBuf.getShort(), sub, 1));
        mList.add(new EmInfo("ecio", mRespByteBuf.getShort(), sub, 1));
    }

    private void setGsmNCellInfo(int sub) {
        long cellId = PrimitiveParser.toUnsigned(mRespByteBuf.getInt());
        mList.add(new EmInfo("gsmncell" + cellId + ":", cellId, sub, 1));
        mList.add(new EmInfo("bcch", PrimitiveParser.toUnsigned(
                mRespByteBuf.getShort()), sub, 1));
        mList.add(new EmInfo("bsic", PrimitiveParser.toUnsigned(mRespByteBuf.get()), sub, 1));
        mList.add(new EmInfo("rxlev", PrimitiveParser.toUnsigned(
                mRespByteBuf.getShort()), sub, 1));
    }

    private void setCommonBaseInfo(int sub) {
        String[] names = {"srv_status", "srv_domain", "sys_mode",
                "roam_status", "mcc", "mnc"};
        for (int i = 0; i < names.length; i++ ) {
            mList.add(new EmInfo(names[i], PrimitiveParser.toUnsigned(
                    mRespByteBuf.getInt()), sub));
        }
    }

    private void setGsmInfo(int sub) {
        mList.add(new EmInfo("lac_id", PrimitiveParser.toUnsigned(mRespByteBuf.getShort()), sub));
        mList.add(new EmInfo("rssi", mRespByteBuf.get(), sub));
        mList.add(new EmInfo("bcch", PrimitiveParser.toUnsigned(mRespByteBuf.getShort()), sub));
        mList.add(new EmInfo("bsic", PrimitiveParser.toUnsigned(mRespByteBuf.get()), sub));

        String[] names = {"rxlev", "rxqual_full", "rxqual_sub", "ta"};
        for (int i=0; i < names.length; i++ ) {
            mList.add(new EmInfo(names[i], PrimitiveParser.toUnsigned(
                    mRespByteBuf.getShort()), sub));
        }

        int gnCellNum = mRespByteBuf.getInt();
        if (gnCellNum <= 0 || gnCellNum > MAX_CELLS) {
            log("invalid gnCellNum:" + gnCellNum);
            return;
        }
        for (int i = 1; i <= gnCellNum; i++) {
            setGsmNCellInfo(sub);
        }
    }

    private void setCdmaInfo(int sub) {
        mList.add(new EmInfo("cdma_rssi", mRespByteBuf.get(), sub));
        mList.add(new EmInfo("cdma_ecio", mRespByteBuf.getShort(), sub));

        String[] names = {"cdma_1x_rx0_agc", "cdma_1x_rx1_agc",
                "cdma_evdo_rx0_agc", "cdma_evdo_rx1_agc"};
        for (int i=0; i < names.length; i++ ) {
            mList.add(new EmInfo(names[i], (int)mRespByteBuf.getFloat(), sub));
        }
    }

    private void setWcdmaInfo(int sub) {
        mList.add(new EmInfo("lac_id", PrimitiveParser.toUnsigned(mRespByteBuf.getShort()), sub));
        mList.add(new EmInfo("dl_bler", PrimitiveParser.toUnsigned(mRespByteBuf.getShort()), sub));
        mList.add(new EmInfo("w_ecio", mRespByteBuf.getShort(), sub));
        mList.add(new EmInfo("w_rscp", mRespByteBuf.getShort(), sub));
        mList.add(new EmInfo("w_agc", (int)mRespByteBuf.getFloat(), sub));
        mList.add(new EmInfo("w_txagc", (int)mRespByteBuf.getFloat(), sub));
        mList.add(new EmInfo("w_dl_uarfcn", PrimitiveParser.toUnsigned(
                mRespByteBuf.getShort()), sub));
        mList.add(new EmInfo("w_psc", PrimitiveParser.toUnsigned(mRespByteBuf.getShort()), sub));

        int wnCellNum = mRespByteBuf.getInt();
        if (wnCellNum <= 0 || wnCellNum > MAX_CELLS) {
            log("invalid wnCellNum:" + wnCellNum);
            return;
        }
        for (int i = 1; i <= wnCellNum; i++) {
            setWcdmaNCellInfo(sub);
        }
    }

    private void insertEmInfo(int sub){
        mSqliteDatabase.beginTransaction();
        ContentValues values = new ContentValues();
        log("insert eminfo size:" + mList.size());
        mSqliteDatabase.delete("eminfo", "mSub = ?", new String[]
                {Integer.toString(sub)});
        for (int i=0; i<mList.size();i++) {
            values.put("mName", mList.get(i).mName);
            values.put("mValue", mList.get(i).mValue);
            values.put("mSub", mList.get(i).mSub);
            values.put("mIsNotImportant", mList.get(i).mIsNotImportant);

            mSqliteDatabase.insert("eminfo", null, values);
        }
        mSqliteDatabase.setTransactionSuccessful();
        mSqliteDatabase.endTransaction();
    }

    protected void log(String s) {
        android.util.Log.d(LOG_TAG, s);
    }
}
