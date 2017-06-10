/**
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.io.File;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

public class RemoteActivity extends Activity implements OnItemClickListener {

    private static final int DIALOG_SETTINGS = 1;

    private static final int DIALOG_INSTALL_LAST = 4;

    private EditText serverEdit;

    private static final String TAG = "RemoteActivity";

    private static final boolean DEBUG = true;

    private ListView updateList;

    private TextView emptyText;

    private LinearLayout rootView;

    private String lastUpdatePath;
    private boolean isLastUpdateDelta;

    private DownloadManager downloadManager;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.remote_updates);

        updateList = (ListView) findViewById(R.id.update_list);
        updateList.setOnItemClickListener(this);
        emptyText = (TextView) findViewById(R.id.empty_text);
        rootView = (LinearLayout) findViewById(R.id.root);

        downloadManager = DownloadManager.getDefault(this);

        lastUpdatePath = UpdateUtil.getLastUpdate(this);
        isLastUpdateDelta = UpdateUtil.getLastIsDelta(this);
        if (lastUpdatePath == null) {
            new ListUpdatesTask().execute(downloadManager.getUpdateListUrl());
        } else {
            showDialog(DIALOG_INSTALL_LAST);
        }
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = new Intent(this, UpdateViewActivity.class);
        intent.putExtra(Intent.EXTRA_INTENT, (UpdateInfo) arg1.getTag());
        startActivity(intent);
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu, menu);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.refresh:
                new ListUpdatesTask().execute(downloadManager.getUpdateListUrl());
                break;
            case R.id.settings:
                showDialog(DIALOG_SETTINGS);
                break;

        }
        return super.onOptionsItemSelected(item);
    }

    private class SetDialogListener implements OnClickListener {

        public void onClick(DialogInterface dialog, int which) {
            downloadManager.saveServerUrl(serverEdit.getText().toString());
        }

    }

    protected void onPrepareDialog(int id, Dialog dialog) {
        super.onPrepareDialog(id, dialog);
        switch (id) {
            case DIALOG_SETTINGS: {
                String url = downloadManager.getServerUrl();
                if (url != null && serverEdit != null) {
                    serverEdit.setText(url);
                }
                break;
            }
        }
    }

    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_SETTINGS: {
                View dialogView = LayoutInflater.from(this).inflate(R.layout.remote_settings, null);
                String url = downloadManager.getServerUrl();
                serverEdit = (EditText) dialogView.findViewById(R.id.server_url);
                if (url != null) {
                    serverEdit.setText(url);
                }
                return new AlertDialog.Builder(this).setView(dialogView)
                        .setTitle(getString(R.string.title_server_url))
                        .setNeutralButton(android.R.string.cancel, null)
                        .setPositiveButton(android.R.string.ok, new SetDialogListener()).create();
            }
            case DIALOG_INSTALL_LAST:
                final File update = new File(lastUpdatePath);
                return new AlertDialog.Builder(this).setTitle(update.getName())
                        .setMessage(R.string.msg_install_last)
                        .setNeutralButton(android.R.string.cancel, new OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {
                                UpdateUtil.deteteUpdate(RemoteActivity.this);
                                new ListUpdatesTask().execute(downloadManager.getUpdateListUrl());
                            }
                        }).setPositiveButton(android.R.string.ok, new OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {
                                UpdateUtil.deteteUpdate(RemoteActivity.this);
                                Intent intent = new Intent(
                                        isLastUpdateDelta ? InstallReceiver.ACTION_REBOOT_DELTA
                                                : InstallReceiver.ACTION_REBOOT);
                                intent.setData(Uri.fromFile(update));
                                startActivity(intent);
                            }
                        }).create();
        }
        return null;
    }

    private class ListUpdatesTask extends AsyncTask<String, Object, List<UpdateInfo>> implements
            OnCancelListener {

        private ProgressDialog mProgressDialog;

        protected void onPostExecute(List<UpdateInfo> result) {
            if (mProgressDialog != null && mProgressDialog.isShowing()) {
                mProgressDialog.dismiss();
            }
            if (this.isCancelled()) {
                return;
            }

            if (result != null && result.size() > 0) {
                rootView.setGravity(Gravity.TOP);
                updateList.setVisibility(View.VISIBLE);
                emptyText.setVisibility(View.GONE);
                updateList.setAdapter(new UpdateListAdapter(RemoteActivity.this, result));
            } else {
                log("there is no update in server");
                rootView.setGravity(Gravity.CENTER);
                updateList.setVisibility(View.GONE);
                emptyText.setVisibility(View.VISIBLE);
            }
        }

        @Override
        protected void onPreExecute() {
            mProgressDialog = new ProgressDialog(RemoteActivity.this);
            mProgressDialog.setTitle(R.string.title_getupdate);
            mProgressDialog.setMessage(getResources().getString(R.string.msg_getupdate));
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            mProgressDialog.setOnCancelListener(this);
            mProgressDialog.show();
        }

        @Override
        protected List<UpdateInfo> doInBackground(String... params) {
            List<UpdateInfo> results = UpdateUtil.getUpdateInfo(params[0]);
            UpdateUtil.updateFileSize(RemoteActivity.this, results);
            return results;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            this.cancel(true);
        }

    }

    private class UpdateListAdapter extends BaseAdapter {

        private Context mContext;

        private List<UpdateInfo> lists;

        public UpdateListAdapter(Context context, List<UpdateInfo> updates) {
            mContext = context;
            lists = updates;
        }

        public int getCount() {
            return lists.size();
        }

        public Object getItem(int arg0) {
            return lists.get(arg0);
        }

        public long getItemId(int arg0) {
            return arg0;
        }

        public View getView(int arg0, View arg1, ViewGroup arg2) {
            View updateInfoView = null;
            if (arg1 != null) {
                updateInfoView = arg1;
            } else {
                updateInfoView = LayoutInflater.from(mContext).inflate(
                        R.layout.update_item, null);
            }
            UpdateInfo updateInfo = (UpdateInfo) getItem(arg0);

            TextView updateNameText = (TextView) updateInfoView.findViewById(R.id.update_file);
            TextView descriptionText = (TextView) updateInfoView.findViewById(R.id.description);
            TextView fileSizeText = (TextView) updateInfoView.findViewById(R.id.size);

            updateNameText.setText(updateInfo.getFileName());
            descriptionText.setText(updateInfo.getDescription());
            fileSizeText.setText(UpdateUtil.formatSize(updateInfo.getSize()));

            updateInfoView.setTag(updateInfo);
            return updateInfoView;
        }

    }

    private void log(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
