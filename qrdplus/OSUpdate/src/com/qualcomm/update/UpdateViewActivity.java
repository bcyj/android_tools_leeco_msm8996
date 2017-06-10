/**
 * Copyright (c) 2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

public class UpdateViewActivity extends Activity implements OnClickListener {

    private UpdateInfo updateInfo;
    private TextView nameView;
    private TextView detailView;
    private Button okBtn;
    private Button cancelBtn;
    private ProgressBar progressBar;
    private LinearLayout progressLayout;
    private TextView progressText;

    private BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DownloadManager.ACTION_STATE_CHANGED.equals(intent.getAction())) {
                updateOkText();
            } else if (DownloadManager.ACTION_PROGRESS_CHANGED.equals(intent.getAction())) {
                int progress = intent.getIntExtra(Intent.EXTRA_INTENT, 0);
                progressBar.setIndeterminate(false);
                progressBar.setProgress(progress);
                progressText.setText(progress + "%");
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.update_info);
        updateInfo = (UpdateInfo) getIntent().getSerializableExtra(Intent.EXTRA_INTENT);

        progressBar = (ProgressBar) findViewById(R.id.download_prog);
        progressLayout = (LinearLayout) findViewById(R.id.download_container);
        progressText = (TextView) findViewById(R.id.download_text);
        nameView = (TextView) findViewById(R.id.file_name);
        detailView = (TextView) findViewById(R.id.update_detail);
        okBtn = (Button) findViewById(R.id.ok_button);
        cancelBtn = (Button) findViewById(R.id.cancel_button);
        okBtn.setOnClickListener(this);
        cancelBtn.setOnClickListener(this);

        IntentFilter filter = new IntentFilter();
        filter.addAction(DownloadManager.ACTION_PROGRESS_CHANGED);
        filter.addAction(DownloadManager.ACTION_STATE_CHANGED);
        registerReceiver(receiver, filter);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        updateInfo = (UpdateInfo) intent.getSerializableExtra(Intent.EXTRA_INTENT);
        updateDetail();
        updateOkText();
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateDetail();
        updateOkText();
    }

    private void updateOkText() {
        if (!DownloadManager.getDefault(this).isDownloading()) {
            okBtn.setText(R.string.btn_download);
            okBtn.setEnabled(true);
            progressLayout.setVisibility(View.INVISIBLE);
        } else if (DownloadManager.getDefault(this).isDownloading(updateInfo)) {
            okBtn.setText(R.string.btn_pause);
            okBtn.setEnabled(true);
            progressLayout.setVisibility(View.VISIBLE);
            progressBar.setIndeterminate(true);
        } else {
            okBtn.setText(R.string.btn_download);
            okBtn.setEnabled(false);
            progressLayout.setVisibility(View.INVISIBLE);
        }
    }

    private void updateDetail() {
        nameView.setText(updateInfo.getFileName());
        detailView.setText(getString(R.string.info_size,
                UpdateUtil.formatSize(updateInfo.getSize()))
                + "\n"
                + getString(R.string.info_version, updateInfo.getVersion())
                + "\n\n"
                + (updateInfo.getDelta() != null ? (getString(R.string.info_delta,
                        String.valueOf(updateInfo.getDelta().from),
                        String.valueOf(updateInfo.getDelta().to)) + "\n") : "")
                + updateInfo.getDescription());
    }

    @Override
    public void onClick(View v) {
        if (v == cancelBtn) {
            finish();
        } else if (v == okBtn) {
            if (!DownloadManager.getDefault(this).isDownloading()) {
                DownloadManager.getDefault(this).download(updateInfo);
            } else if (DownloadManager.getDefault(this).isDownloading(updateInfo)) {
                DownloadManager.getDefault(this).pause(updateInfo);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(receiver);
    }

}
