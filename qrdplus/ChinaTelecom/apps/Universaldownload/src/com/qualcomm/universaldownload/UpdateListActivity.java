/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class UpdateListActivity extends Activity {

    private static final String TAG = "UpdateListActivity";
    private static final int MSG_GO_TO_LIST = 0;
    private static final int MSG_UPDATE_PROGRESS = 1;
    private static final int MSG_SHOW_NET_ERROR = 2;

    private static final int DOWNLOAD_COMPLETE = 1;
    private static final int NETWORK_ERROR = 2;
    private static final int NO_UPDATE_INFO = 3;
    private DownloadSharePreference mPref;
    private LinearLayout mRoot;
    private RelativeLayout updateListLayout;
    private ArrayList<UpdateInfo> mUpdateInfos = new ArrayList<UpdateInfo>();
    private RelativeLayout mConfirmLayout;
    private RelativeLayout mDownloadingLayout;
    private ProgressBar mDownProgress;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch(msg.what) {
                case MSG_GO_TO_LIST:
                    goUpdateList();
                    break;
                case MSG_UPDATE_PROGRESS:
                    mDownProgress.setProgress(msg.arg1);
                    break;
                case MSG_SHOW_NET_ERROR:
                    showNetErrorDialog();
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.update);
        mRoot = (LinearLayout) this.findViewById(R.id.root);
        mPref = DownloadSharePreference.Instance(this);
        Log.e(TAG, "oncreate bind to service");
        Intent intent = new Intent(this, DownloadService.class);
        startService(intent);
        bindService(intent, connectservice, 0);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(connectservice);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        switch(id) {
            case DOWNLOAD_COMPLETE:
                builder.setIcon(R.drawable.ic_launcher).
                        setMessage(R.string.download_complete).
                        setTitle(R.string.app_name).
                        setPositiveButton(R.string.OK, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                finish();
                            }
                        }).setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialogInterface) {
                        finish();
                    }
                });
                return builder.create();

            case NETWORK_ERROR:
                builder.setIcon(R.drawable.ic_launcher).
                        setMessage(R.string.network_busy).
                        setTitle(R.string.app_name).
                        setPositiveButton(R.string.OK, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                finish();
                            }
                        }).setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialogInterface) {
                        finish();
                    }
                });
                return builder.create();

            case NO_UPDATE_INFO:
                builder.setIcon(R.drawable.ic_launcher).
                        setMessage(R.string.no_update_info).
                        setTitle(R.string.app_name).
                        setPositiveButton(R.string.back, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                finish();
                            }
                        }).setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialogInterface) {
                        finish();
                    }
                });
                return builder.create();
        }
        return super.onCreateDialog(id);
    }

    private IDownloadService mService = null;
    private ServiceConnection connectservice = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            Log.d(TAG, "onServiceConnected ibinder is " +  iBinder);
            mService = (IDownloadService) iBinder;
            try {
                if(mService != null) {
                    mService.registerListener(mDownloadListener);
                    mService.checkUpdate(mPref.getServerAddr());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {

        }
    };

    private IDownloadListener mDownloadListener = new IDownloadListener.Stub() {
        @Override
        public void onUpdateComplete(boolean success) throws RemoteException {
            if(success) {
                mHandler.sendEmptyMessage(MSG_GO_TO_LIST);
            } else {
                mHandler.sendEmptyMessage(MSG_SHOW_NET_ERROR);
            }
        }

        @Override
        public void onDownloadProgressUpdate(int progress) throws RemoteException {
            mHandler.sendMessage(mHandler.obtainMessage(MSG_UPDATE_PROGRESS, progress, 0));
        }

        @Override
        public void onOneFileComplete(final int num) throws RemoteException {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    TextView downInfo = (TextView)
                            mDownloadingLayout.findViewById(R.id.download_info);
                    String downloadText = getString(R.string.downloading);
                    downInfo.setText(downloadText + num + "/" + mUpdateInfos.size());
                }
            });
        }

        @Override
        public void onAllComplete() throws RemoteException {
            mHandler.post( new Runnable() {
                @Override
                public void run() {
                    showDialog(DOWNLOAD_COMPLETE);
                }
            });
        }
    };

    private void showNetErrorDialog() {
        showDialog(NETWORK_ERROR);
    }

    private void goUpdateList() {
        getUpdatedInfos();
        mRoot.removeAllViews();
        if(mUpdateInfos.size() == 0) {
            showDialog(NO_UPDATE_INFO);
            return;
        }
        updateListLayout = (RelativeLayout) View.inflate(this, R.layout.updatelist, null);
        ListView list = (ListView) updateListLayout.findViewById(R.id.updatelist);
        list.setAdapter(new updateListAdapter(mUpdateInfos));
        mConfirmLayout = (RelativeLayout) updateListLayout.findViewById(R.id.confirm_down_layout);
        mConfirmLayout.setVisibility(View.VISIBLE);
        mDownloadingLayout = (RelativeLayout) updateListLayout.
                findViewById(R.id.downloading_layout);
        mDownloadingLayout.setVisibility(View.GONE);
        Button btnCancel = (Button) mConfirmLayout.findViewById(R.id.btn_cancel);
        Button btnDownload = (Button) mConfirmLayout.findViewById(R.id.btn_download);
        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finish();
            }
        });
        btnDownload.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startDownload();
            }
        });

        mDownProgress = (ProgressBar) mDownloadingLayout.findViewById(R.id.down_progressBar);
        mDownProgress.setProgress(0);
        mRoot.addView(updateListLayout);
    }

    private void startDownload() {
        mConfirmLayout.setVisibility(View.GONE);
        mDownloadingLayout.setVisibility(View.VISIBLE);
        Button btnCancelDown = (Button) mDownloadingLayout.findViewById(R.id.btn_cancel_down);
        btnCancelDown.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    if(mService != null) {
                        mService.cancelDownload();
                    }
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                finish();
            }
        });
        try {
            if(mService != null) {
                mService.downloadData(mUpdateInfos);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void getUpdatedInfos() {
        try {
            for(UpdateItem item : mPref.getCheckedItem()) {
                UpdateInfo info = mService.getUpdateInfo(item.getCapabilityName());
                if(info != null) {
                    if(item.getVersion().equals("") ||
                            info.getVersion().compareToIgnoreCase(item.getVersion()) > 0) {
                        mUpdateInfos.add(info);
                    }
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private class updateListAdapter extends BaseAdapter {
        List<UpdateInfo> mInfos;
        public updateListAdapter(ArrayList<UpdateInfo> infos) {
            mInfos = infos;
        }
        @Override
        public int getCount() {
            return mInfos.size();
        }

        @Override
        public Object getItem(int i) {
            return mInfos.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {
            UpdateInfo info = mInfos.get(i);
            UpdateListItem item;
            if(view instanceof UpdateListItem) {
                item = (UpdateListItem) view;
            } else {
                item = new UpdateListItem(UpdateListActivity.this);
            }
            item.setUpdateTitle(getCapabilityLabel(info.getCapability()));
            String new_version = getString(R.string.new_version);
            item.setUpdateVersion(new_version + info.getVersion());
            return item;
        }
    }

    private String getCapabilityLabel(String name) {
        String[] updateLabels = getResources().getStringArray(R.array.pref_update_items_label);
        for(int i = 0; i < updateLabels.length; i++) {
            if(updateLabels[i++].equals(name)) {
                return  updateLabels[i];
            }
        }
        return null;
    }
}
