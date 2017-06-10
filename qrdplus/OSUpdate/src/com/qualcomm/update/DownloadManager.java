/**
 * Copyright (c) 2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import javax.net.ssl.HttpsURLConnection;
import java.net.URL;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;

public class DownloadManager {
    private static final boolean DEBUG = true;
    private static final String TAG = "updateDownload";

    private static DownloadManager instance;

    private NotificationManager nManager;
    private SharedPreferences sp;
    private Context mContext;

    /**
     * action for state changed
     */
    static final String ACTION_STATE_CHANGED = "com.qualcomm.update.DOWNLOAD_STATE_CHANGED";

    /**
     * action for progress changed
     */
    static final String ACTION_PROGRESS_CHANGED = "com.qualcomm.update.DOWNLOAD_PROGRESS_CHANGED";

    /**
     * current task
     */
    private DownloadTask task;

    private static final int WHAT_TASK_PROGRESS = 1;
    private static final int WHAT_TASK_COMPLETE = 2;
    private static final int WHAT_TASK_FAILED = 3;
    private static final int WHAT_TASK_PAUSE = 4;
    private static final int WHAT_TASK_START = 5;

    private static final String KEY_REMOTE = "url_server";

    private static final String KEY_TAG_REMOTE_URL = "tag_remote_url";
    private static final String KEY_TAG_UPDATE_SIZE = "tag_update_size";
    private static final String KEY_TAG_LOCAL_PATH = "tag_local_path";
    private static final String KEY_TAG_LOCAL_SIZE = "tag_local_size";
    private static final String KEY_TAG_LOCAL_TIME = "tag_local_time";

    private static final String PATH_SAVE_DIR = Environment.getExternalStorageDirectory()
            .getAbsolutePath() + "/QRDUpdate/";

    private Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case WHAT_TASK_PROGRESS:
                    notifyProgress(msg.arg1);
                    break;
                case WHAT_TASK_COMPLETE:
                    File f = (File) msg.getData().getSerializable("file");
                    boolean delta = msg.getData().getBoolean("delta");
                    notifyComplete(f, delta);
                    break;
                case WHAT_TASK_FAILED:
                    notifyFailed();
                    break;
                case WHAT_TASK_PAUSE:
                    notifyPause();
                    break;
                case WHAT_TASK_START:
                    notifyStart();
                    break;
            }
        }

    };

    private DownloadManager(Context context) {
        nManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        sp = PreferenceManager.getDefaultSharedPreferences(context);
        mContext = context;
        UpdateUtil.mkDeltaDir(mContext);
    }

    /**
     * notify the state changed
     */
    private void notifyStateChanged() {
        mContext.sendBroadcast(new Intent(ACTION_STATE_CHANGED));
    }

    /**
     * notify task start
     */
    protected void notifyStart() {
        nManager.notify(0, new DownloadNotification(0, true));
    }

    /**
     * notify the task pause
     */
    protected void notifyPause() {
        nManager.cancel(0);
        Toast.makeText(mContext, R.string.toast_task_pause, Toast.LENGTH_SHORT).show();
    }

    /**
     * notify the task failed
     */
    protected void notifyFailed() {
        nManager.cancel(0);
        Toast.makeText(mContext, R.string.toast_task_failed, Toast.LENGTH_SHORT).show();
    }

    /**
     * notify the task download success
     */
    protected void notifyComplete(File file, boolean delta) {
        nManager.cancel(0);
        Intent intent = new Intent(delta ? InstallReceiver.ACTION_REBOOT_DELTA
                : InstallReceiver.ACTION_REBOOT);
        intent.setData(Uri.fromFile(file));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    /**
     * notify the task progress
     *
     * @param progress
     */
    protected void notifyProgress(int progress) {
        nManager.notify(0, new DownloadNotification(progress));
        Intent intent = new Intent(ACTION_PROGRESS_CHANGED);
        intent.putExtra(Intent.EXTRA_INTENT, progress);
        mContext.sendBroadcast(intent);
    }

    public static DownloadManager getDefault(Context context) {
        if (instance == null) {
            instance = new DownloadManager(context);
        }
        return instance;
    }

    /**
     * is there a task running
     *
     * @return
     */
    boolean isDownloading() {
        return task != null;
    }

    /**
     * is the update file downloading now
     *
     * @param updateInfo
     * @return
     */
    boolean isDownloading(UpdateInfo updateInfo) {
        return task != null
                && TextUtils.equals(task.update.getFileName(), updateInfo.getFileName());
    }

    /**
     * return the url to download the file
     *
     * @param updateInfo
     * @return
     */
    private String getDownloadUrl(UpdateInfo updateInfo) {
        String serverUrl = getServerUrl();
        if (serverUrl == null)
            return null;
        else {
            return serverUrl + "/" + updateInfo.getFileName();
        }
    }

    /**
     * return server url
     *
     * @return
     */
    public String getServerUrl() {
        return sp.getString(KEY_REMOTE, null);
    }

    /**
     * save server url
     *
     * @param url
     * @return
     */
    public boolean saveServerUrl(String url) {
        return sp.edit().putString(KEY_REMOTE, url).commit();
    }

    /**
     * return the url for all update files
     *
     * @return
     */
    public String getUpdateListUrl() {
        String serverUrl = getServerUrl();
        if (serverUrl == null)
            return null;
        return serverUrl + "/updates.xml";
    }

    /**
     * download the file
     *
     * @param updateInfo
     */
    void download(UpdateInfo updateInfo) {
        if (isDownloading()) {
            throw new IllegalArgumentException("there is a task downloading now!");
        }
        task = new DownloadTask(updateInfo);
        notifyStateChanged();
        task.execute();
    }

    /**
     * pause to download the file
     *
     * @param updateInfo
     */
    void pause(UpdateInfo updateInfo) {
        if (!isDownloading(updateInfo)) {
            throw new IllegalArgumentException("the lask has not started!");
        }
        task.cancel(true);
    }

    /**
     * save tag to local used for continue download
     *
     * @param path
     * @param info
     * @return
     */
    private boolean saveTag(String path, UpdateInfo info) {
        File f = new File(path);
        Editor editor = sp.edit();
        editor.putString(KEY_TAG_REMOTE_URL, getDownloadUrl(info));
        editor.putLong(KEY_TAG_UPDATE_SIZE, info.getSize());
        editor.putString(KEY_TAG_LOCAL_PATH, path);
        editor.putLong(KEY_TAG_LOCAL_SIZE, f.length());
        editor.putLong(KEY_TAG_LOCAL_TIME, f.lastModified());
        return editor.commit();
    }

    /**
     * clear tag
     *
     * @return
     */
    private boolean clearTag() {
        Editor editor = sp.edit();
        editor.remove(KEY_TAG_REMOTE_URL);
        editor.remove(KEY_TAG_UPDATE_SIZE);
        editor.remove(KEY_TAG_LOCAL_PATH);
        editor.remove(KEY_TAG_LOCAL_SIZE);
        editor.remove(KEY_TAG_LOCAL_TIME);
        return editor.commit();
    }

    /**
     * get the tag last time download
     *
     * @param path
     * @param info
     * @return
     */
    private long getTag(String path, UpdateInfo info) {
        File f = new File(path);
        if (f.exists()
                && TextUtils.equals(getDownloadUrl(info), sp.getString(KEY_TAG_REMOTE_URL, null))
                && sp.getLong(KEY_TAG_UPDATE_SIZE, 0) == info.getSize()
                && TextUtils.equals(path, sp.getString(KEY_TAG_LOCAL_PATH, null))
                && f.length() == sp.getLong(KEY_TAG_LOCAL_SIZE, 0)
                && f.lastModified() == sp.getLong(KEY_TAG_LOCAL_TIME, 0)) {
            return f.length();
        }
        return -1;
    }

    class DownloadTask extends AsyncTask<Object, Object, File> {

        private UpdateInfo update;
        private float total;
        private float read;

        DownloadTask(UpdateInfo updateInfo) {
            update = updateInfo;
        }

        @Override
        protected void onCancelled() {
            super.onCancelled();
            if (task != null) {
                task = null;
                notifyStateChanged();
            }
        }

        @Override
        protected void onPostExecute(File result) {
            super.onPostExecute(result);
            if (result != null) {
                Bundle data = new Bundle();
                data.putBoolean("delta", update.getDelta() != null);
                data.putSerializable("file", result);
                Message message = handler.obtainMessage(WHAT_TASK_COMPLETE);
                message.setData(data);
                message.sendToTarget();
            }
            if (task != null) {
                task = null;
                notifyStateChanged();
            }
        }

        @Override
        protected File doInBackground(Object... params) {
            handler.sendEmptyMessage(WHAT_TASK_START);
            String url = getDownloadUrl(update);
            String local = PATH_SAVE_DIR + update.getFileName();

            HttpsURLConnection connection = null;
            InputStream is = null;
            FileOutputStream os = null;
            URL remoteUrl = null;
            byte buffer[] = new byte[4096];
            int readsize = 0;
            File file = new File(local);
            file.getParentFile().mkdirs();
            long tag = getTag(local, update);
            if (DEBUG) {
                Log.d(TAG, "download file: " + url);
                Log.d(TAG, "file path to save: " + local);
                Log.d(TAG, "get tag: " + tag);
            }
            try {
                // the file has download last time
                if (tag == update.getSize()) {
                    return file;
                }

                if (!file.exists()) {
                    file.createNewFile();
                }
                remoteUrl = new URL(url);
                connection = (HttpsURLConnection) remoteUrl.openConnection();
                connection.setRequestProperty("User-Agent", "PacificHttpClient");
                if (tag != -1) {
                    connection.setRequestProperty("RANGE", "bytes=" + tag + "-");
                }
                connection.setConnectTimeout(30000);
                connection.setReadTimeout(20000);
                if (connection.getResponseCode() == 404) {
                    handler.sendEmptyMessage(WHAT_TASK_FAILED);
                    Log.w(TAG, "get http response 404!");
                    return null;
                }
                total = connection.getContentLength() + (tag != -1 ? tag : 0);
                // not support for pause and restart
                if (tag != -1 && total != update.getSize()) {
                    total = update.getSize();
                    tag = -1;
                    clearTag();
                }
                if (DEBUG) {
                    Log.d(TAG, "total length of the file: " + total);
                }
                is = connection.getInputStream();
                os = new FileOutputStream(file, tag != -1);
                read = tag != -1 ? tag : 0;
                int last = 0;
                int pro = (int) (read / total * 100);
                handler.obtainMessage(WHAT_TASK_PROGRESS, pro, 0).sendToTarget();
                while ((readsize = is.read(buffer)) > 0 && !isCancelled()) {
                    read += readsize;
                    pro = (int) (read / total * 100);
                    if (pro > last) {
                        handler.obtainMessage(WHAT_TASK_PROGRESS, pro, 0).sendToTarget();
                        last = pro;
                    }
                    os.write(buffer, 0, readsize);
                    os.flush();
                }
                handler.obtainMessage(WHAT_TASK_PROGRESS, pro, 0).sendToTarget();
                // save the tag for the data has download
                saveTag(local, update);
                if (isCancelled()) {
                    handler.sendEmptyMessage(WHAT_TASK_PAUSE);
                    return null;
                }
            } catch (Exception e) {
                Log.w(TAG, "failed to download the file! " + e);
                handler.sendEmptyMessage(WHAT_TASK_FAILED);
                return null;
            } finally {
                if (os != null) {
                    try {
                        os.close();
                    } catch (IOException e) {
                    }
                }
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }
                }

                if (connection != null) {
                    connection.disconnect();
                }
            }
            return file;
        }
    }

    class DownloadNotification extends Notification {

        DownloadNotification(int progress) {
            this(progress, false);
        }

        DownloadNotification(int progress, boolean indeterminate) {
            Intent intent = new Intent(mContext, UpdateViewActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(Intent.EXTRA_INTENT, task.update);
            contentIntent = PendingIntent.getActivity(mContext, 0, intent, 0);
            icon = android.R.drawable.stat_sys_download;
            tickerText = task.update.getFileName();
            contentView = new RemoteViews(mContext.getPackageName(), R.layout.download_notification);
            contentView.setProgressBar(R.id.download_prog, 100, progress, indeterminate);
            contentView.setTextViewText(R.id.file_name, task.update.getFileName());
            contentView.setTextViewText(R.id.prog_text, UpdateUtil.formatSize(task.read) + "/"
                    + UpdateUtil.formatSize(task.total));
            flags |= Notification.FLAG_NO_CLEAR;
        }
    }
}
