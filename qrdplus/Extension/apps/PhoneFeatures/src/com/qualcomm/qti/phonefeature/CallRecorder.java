/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import android.content.Context;
import android.content.Intent;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.StatFs;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;

public class CallRecorder extends Handler {

    private static final String TAG = "CallRecorder";

    private static final String ACTION_RECORD_STATE_CHANGED =
            "com.qualcomm.qti.phonefeature.RECORD_STATE_CHANGED";

    private static final long LOW_STORAGE_THRESHOLD = 50000000;
    private static final String RECORD_FILE_SUBFIX = ".amr";
    private static String sCallRecordPath = Environment.getExternalStorageDirectory()
            + File.separator + "CallRecord";

    private static final int EVENT_START = 1;
    private static final int EVENT_STOP = 2;
    private static final int EVENT_CALL_STATE_CHANGED = 3;

    private static CallRecorder sCallRecorder;

    private CallManager mCM = CallManager.getInstance();
    private final Context mContext;
    private File mTarget;
    private MediaRecorder mMediaRecorder;
    private long mStartTimeMillis;
    private Connection mConOnRec;
    private MediaRecorder.OnInfoListener mOnInfoListener = new MediaRecorder.OnInfoListener() {
        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            if (what == MediaRecorder.MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED) {
                obtainMessage(EVENT_STOP).sendToTarget();
            }
        }
    };

    private CallRecorder(Context context) {
        super(Looper.getMainLooper());
        mContext = context;
        mCM.registerForPreciseCallStateChanged(this, EVENT_CALL_STATE_CHANGED, null);
        if (mContext.getResources().getBoolean(R.bool.def_save_name_prefix_enabled)) {
            String callRecordSavePath = mContext.getResources()
                    .getString(R.string.def_callRecord_savePath);
            sCallRecordPath = Environment.getExternalStorageDirectory()
            + File.separator + callRecordSavePath;
        }
    }

    public static CallRecorder getInstance(Context context) {
        synchronized (CallRecorder.class) {
            if (sCallRecorder == null) {
                sCallRecorder = new CallRecorder(context);
            }
            return sCallRecorder;
        }
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_START:
                startRecord();
                break;
            case EVENT_STOP:
                stopRecord(false);
                break;
            case EVENT_CALL_STATE_CHANGED:
                onPhoneStateChanged();
                break;
        }
    }

    private void notifyRecordStateChanged() {
        Log.d(TAG, "notify record state changed, " + isRecording());
        mContext.sendBroadcast(new Intent(ACTION_RECORD_STATE_CHANGED));
    }

    private void onPhoneStateChanged() {
        if (!isRecording()) {
            return;
        }
        if (!isAvailable()) {
            stopRecord(false);
        }
        if (mConOnRec == null || !mConOnRec.equals(mCM.getFgCallLatestConnection())) {
            stopRecord(false);
        }
    }

    public void start() {
        obtainMessage(EVENT_START).sendToTarget();
    }

    public void stop() {
        obtainMessage(EVENT_STOP).sendToTarget();
    }

    public long getDuration() {
        if (isRecording()) {
            return System.currentTimeMillis() - mStartTimeMillis;
        }
        return 0;
    }

    public boolean isRecording() {
        return mMediaRecorder != null;
    }

    public boolean isAvailable() {
        boolean isDialing = mCM.getActiveFgCallState().isDialing();
        boolean isAlive = mCM.getActiveFgCallState() == Call.State.ACTIVE;
        int fgConntions = mCM.getActiveFgCall().getConnections().size();
        Log.d(TAG, "isDialing: " + isDialing);
        Log.d(TAG, "isAlive: " + isAlive);
        Log.d(TAG, "fgConntions: " + fgConntions);
        return (isDialing && fgConntions > 1) || isAlive;
    }

    private boolean isStorageFull() {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
        long leftSize = (long) stat.getBlockSizeLong() * stat.getAvailableBlocksLong();
        Log.d(TAG, "external storage left size:" + leftSize);
        return leftSize < LOW_STORAGE_THRESHOLD;
    }

    private long getAvailableSpace() {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
        return stat.getBlockSizeLong() * stat.getAvailableBlocksLong();
    }

    private File createFile() {
        Calendar c = Calendar.getInstance();
        SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd_HHmmss");
        String name = df.format(c.getTime());
        File parent = new File(sCallRecordPath);
        if (!parent.exists() && !parent.mkdirs()) {
            return null;
        }
        int i = 1;
        File f = new File(sCallRecordPath, name + RECORD_FILE_SUBFIX);
        try {
            while (!f.createNewFile()) {
                f = new File(sCallRecordPath, name + "(" + i++ + ")" + RECORD_FILE_SUBFIX);
            }
            return f;
        } catch (IOException e) {
        }
        return null;
    }

    private int init() {
        int errorId = 0;
        if (mMediaRecorder != null) {
            errorId = R.string.message_record_started;
        } else if (!isAvailable()) {
            errorId = R.string.message_not_incall;
        } else if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            errorId = R.string.message_no_sdcard;
        } else if (isStorageFull()) {
            errorId = R.string.message_sdcard_full;
        } else if ((mTarget = createFile()) == null) {
            errorId = R.string.message_internal_error;
        }
        return errorId;
    }

    private void startRecord() {
        int errorMsgId = init();
        if (errorMsgId != 0) {
            Toast.makeText(mContext, errorMsgId, Toast.LENGTH_SHORT).show();
            return;
        }
        long maxFileSize = getAvailableSpace() - LOW_STORAGE_THRESHOLD;
        mMediaRecorder = new MediaRecorder();
        try {
            mMediaRecorder.setMaxFileSize(maxFileSize);
            mMediaRecorder.setOnInfoListener(mOnInfoListener);
            mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.VOICE_CALL);
            mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.RAW_AMR);
            mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
            mMediaRecorder.setOutputFile(mTarget.getAbsolutePath());
            mMediaRecorder.prepare();
            mMediaRecorder.start();
            mConOnRec = mCM.getFgCallLatestConnection();
            mStartTimeMillis = System.currentTimeMillis();
            notifyRecordStateChanged();
        } catch (Exception e) {
            Log.d(TAG, "error when start recording!", e);
            stopRecord(true);
        }
    }

    private void stopRecord(boolean deleteFile) {
        if (mMediaRecorder == null)
            return;
        try {
            mMediaRecorder.stop();
        } catch (IllegalStateException e) {
            Log.d(TAG, "error when stop recording!", e);
        }
        mMediaRecorder.reset();
        mMediaRecorder.release();
        if (!deleteFile) {
            Toast.makeText(mContext,
                    mContext.getString(R.string.message_success, mTarget.getAbsoluteFile()),
                    Toast.LENGTH_SHORT).show();
        } else {
            mTarget.delete();
        }
        Uri uri = Uri.fromFile(mTarget);
        mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri));
        mTarget = null;
        mMediaRecorder = null;
        notifyRecordStateChanged();
    }
}
