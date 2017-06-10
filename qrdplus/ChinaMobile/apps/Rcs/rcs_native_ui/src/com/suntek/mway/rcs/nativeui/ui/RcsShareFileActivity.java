/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */
package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcloudfile.FileNode;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcloudfile.TransNode.TransOper;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcloudfile.TransNode.TransType;
import com.suntek.mway.rcs.client.api.mcloud.McloudFileApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.ui.SaiunFileShareAdapter.ViewHolder;
import com.suntek.mway.rcs.nativeui.utils.ImageUtils;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.client.api.util.FileSuffixException;
import com.suntek.mway.rcs.client.aidl.plugin.callback.IMcloudOperationCtrl;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Looper;
import android.os.Parcelable;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import java.util.ArrayList;
import java.util.Arrays;

public class RcsShareFileActivity extends Activity implements OnClickListener {

    private static final int REQUEST_SELECT_SAIUN_LOCAL_FILE = 11;

    private ArrayList<FileNode> mShareNodeList = new ArrayList<FileNode>();
    private ListView mListView;
    private Context mContext;
    private SaiunFileShareAdapter mSaiunFileShareAdapter;
    private static IMcloudOperationCtrl operation = null;
    private McloudFileApi mMcloudFileApi = null;
    private ProgressDialog mDialog;
    private ProgressDialog mProgressDialog = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = RcsShareFileActivity.this;
        setContentView(R.layout.rcs_share_file_activity);
        register_Receiver();
        initView();
        getRemoteFileList("", -1, 0);
        showWaitDialog(mContext);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private void showWaitDialog(Context context){
        mDialog = ProgressDialog.show(
                context,
                context.getString(R.string.please_wait_moment),
                context.getString(R.string.caiyun_file_is_downing),
                false);
        mDialog.setCanceledOnTouchOutside(true);
    }

    private void getRemoteFileList(String remotePath, int beginIndex, int endIndex) {
        try {
            RcsApiManager.getMcloudFileApi()
                    .getRemoteFileList(remotePath, beginIndex, endIndex, FileNode.Order.createdate);
        } catch (ServiceDisconnectedException e) {
            Log.i("RCS_UI","get remote file list error");
            if (mDialog != null) {
                mDialog.dismiss();
                showSelectContinueOrReturn();
            }
            e.printStackTrace();
        }
    }

    private void initView() {
        mListView = (ListView) findViewById(R.id.saiun_file_list);
        mSaiunFileShareAdapter = new SaiunFileShareAdapter(this);
        mListView.setAdapter(mSaiunFileShareAdapter);
        mSaiunFileShareAdapter.setDatas(mShareNodeList);
        mListView.setOnItemClickListener(mOnItemClickListener);
        mMcloudFileApi = RcsApiManager.getMcloudFileApi();
    }

    private OnItemClickListener mOnItemClickListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position,
                long id) {
            ViewHolder mViewHolder = (ViewHolder) view.getTag();
            FileNode mFileNode = (FileNode) mViewHolder.mFileName.getTag();
            // TODO check if is file
            if (mFileNode.isFile()) {
                Intent intent = new Intent();
                intent.putExtra("id", mFileNode.getId());
                setResult(RESULT_OK, intent);
                finish();
            } else {
                getRemoteFileList(mFileNode.getId(), -1, 0);
            }
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.back_layout:
            finish();
            break;
        case R.id.btn_upload_saiun_file:
            showUploadFileDialog();
            break;
        }
    }

    private void showUploadFileDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(
                RcsShareFileActivity.this);
        builder.setTitle(R.string.rcs_saiun_file_upload_file);
        builder.setMessage(mContext
                .getString(R.string.rcs_saiun_file_upload_marked_words));
        builder.setPositiveButton(R.string.rcs_confirm,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                        intent.setType("*/*");
                        intent.addCategory(Intent.CATEGORY_OPENABLE);
                        startActivityForResult(intent,
                                REQUEST_SELECT_SAIUN_LOCAL_FILE);
                    }
                });
        builder.setNegativeButton(R.string.rcs_cancel, null);
        builder.create().show();
    }

    private void showSelectContinueOrReturn() {
        AlertDialog.Builder builder = new AlertDialog.Builder(
                RcsShareFileActivity.this);
        builder.setTitle(R.string.rcs_storage_manager_tip);
        builder.setMessage(R.string.please_select_continue_or_return);
        builder.setPositiveButton(R.string.rcs_confirm,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                      getRemoteFileList("", -1, 0);
                      mDialog.show();
                    }
                });
        builder.setNegativeButton(R.string.rcs_cancel,new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
              finish();
            }
        });
        builder.create().show();
    }

    private void register_Receiver() {
        Log.i("RCS_UI","register receiver");
        IntentFilter contactfilter = new IntentFilter(
                BroadcastConstants.UI_MC_GET_SHARE_FILE_LIST);
        contactfilter.addAction(BroadcastConstants.UI_MC_PUT_FILE);
        contactfilter.addAction(BroadcastConstants.UI_MC_GET_REMOTE_FILE_LIST);

        this.registerReceiver(receiver, contactfilter);
    }

    private BroadcastReceiver receiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(BroadcastConstants.UI_MC_GET_SHARE_FILE_LIST)) {
                Log.i("RCS_UI","GETlIST="+action);
                String enentType = intent
                        .getStringExtra(BroadcastConstants.BC_VAR_MC_ENENTTYPE);
                Log.i("RCS_UI","GETlISTtYPE="+enentType);
                if (enentType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_SUCCESS)) {
                    Parcelable[] fileNodeList = (Parcelable[]) intent
                            .getParcelableArrayExtra(BroadcastConstants.BC_VAR_MC_SHARE_NODE_LIST);
                    FileNode[] resultArray = null;

                    if (fileNodeList != null) {
                        resultArray = Arrays.copyOf(fileNodeList, fileNodeList.length,
                                FileNode[].class);
                    }
                    LogHelper.trace("fileNodeList = " + resultArray);
                    mShareNodeList.clear();
                    mShareNodeList.addAll(Arrays.asList(resultArray));

                    if (mSaiunFileShareAdapter != null)
                        mSaiunFileShareAdapter.setDatas(mShareNodeList);
                } else if (enentType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_ERROR)) {
                    String error = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_MC_MESSAGE);
                    Log.i("RCS_UI","ERROR="+error);
                    LogHelper.trace(getBaseContext().getResources().getString(
                            R.string.rcs_get_saiun_file_list_fail)
                            + error);
                    Toast.makeText(RcsShareFileActivity.this,
                            R.string.rcs_get_saiun_file_list_fail,
                            Toast.LENGTH_SHORT).show();
                }
            } else if (action.equals(BroadcastConstants.UI_MC_GET_REMOTE_FILE_LIST)) {
                Log.i("RCS_UI","ACTION="+action);
                String enentType = intent
                        .getStringExtra(BroadcastConstants.BC_VAR_MC_ENENTTYPE);
                if (enentType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_SUCCESS)) {
                    Parcelable[] fileNodeList = (Parcelable[]) intent
                            .getParcelableArrayExtra(BroadcastConstants.BC_VAR_MC_REMOTE_NODE_LIST);
                    FileNode[] resultArray = null;
                    if (fileNodeList != null) {
                        resultArray = Arrays.copyOf(fileNodeList, fileNodeList.length,
                                FileNode[].class);
                    }
                    LogHelper.trace("shareNodeList = " + resultArray);
                    mShareNodeList.clear();
                    mShareNodeList.addAll(Arrays.asList(resultArray));
                    if (mSaiunFileShareAdapter != null)
                        mSaiunFileShareAdapter.setDatas(mShareNodeList);
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                } else if (enentType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_ERROR)) {
                    String error = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_MC_MESSAGE);
                    LogHelper.trace(getBaseContext().getResources().getString(
                            R.string.rcs_get_saiun_file_list_fail)
                            + error);
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                    showSelectContinueOrReturn();
                }

            } else if (action.equals(BroadcastConstants.UI_MC_PUT_FILE)) {
                Log.i("RCS_UI","action="+action);
                String eventType = intent
                        .getStringExtra(BroadcastConstants.BC_VAR_MC_ENENTTYPE);
                Log.i("RCS_UI","eventType="+eventType);
                if (eventType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_ERROR)) {
                    String message = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_MC_MESSAGE);
                    LogHelper.trace(getBaseContext().getResources().getString(
                            R.string.rcs_upload_saiun_file_fail)
                            + message);
                    Toast.makeText(RcsShareFileActivity.this,
                            R.string.rcs_upload_saiun_file_fail, Toast.LENGTH_SHORT)
                            .show();
                } else if (eventType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_PROGRESS)) {
                    float percent = (int) intent.getLongExtra(
                            BroadcastConstants.BC_VAR_MC_PROCESS_SIZE, 0);
                    float total = (int) intent.getLongExtra(
                            BroadcastConstants.BC_VAR_MC_TOTAL_SIZE, 0);
                    Log.i("RCS_UI","percent="+percent);
                    showProgressDialog((int)((percent/total)*100));
                } else if (eventType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_SUCCESS)) {
                    if (mProgressDialog != null) {
                        mProgressDialog.cancel();
                        mProgressDialog = null;
                    } else {
                        mProgressDialog = null;
                    }
                    // share
                    String fileLocalPath = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_MC_LOCALPATH);
                    String fullPathInID = intent.getStringExtra(BroadcastConstants.BC_VAR_MC_FULL_FILE_ID);
                    Log.i("RCS_UI","fullPathInID="+fullPathInID);
                    //mShareSaiunFileManager.updateForLocalPath(fileLocalPath);
                    getRemoteFileList("",-1, 0);
                    uploadSucess();
                } else if (eventType
                        .equals(BroadcastConstants.BC_V_MC_EVENTTYPE_FILE_TOO_LARGE)) {
                    long fileMaxSizeKb = intent.getLongExtra(
                            BroadcastConstants.BC_VAR_FILE_MAX_SIZE, 0);
                    String maxMB = ImageUtils.getFileSize(fileMaxSizeKb,
                            1024 * 1024, "MB");
                    Log.i("RCS_UI","MAXmB="+maxMB);
                    LogHelper.trace(getBaseContext().getResources().getString(
                            R.string.rcs_saiun_file_max_size_outstrip)
                            + maxMB);
                    Toast.makeText(
                            RcsShareFileActivity.this,
                            getBaseContext().getResources().getString(
                                    R.string.rcs_saiun_file_max_size_outstrip)
                                    + maxMB, Toast.LENGTH_SHORT).show();
                }
            }
        }
    };

    private void showProgressDialog(int progress) {
        Log.i("RCS_UI","SHOW PROGRESSdIALOG");
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(RcsShareFileActivity.this);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getBaseContext().getResources()
                    .getString(R.string.rcs_saiun_file_uploading_file));
            // mProgressDialog.setCancelable(false);
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setProgress(progress);
            mProgressDialog.setButton(mContext.getString(R.string.pause_upload), new DialogInterface.OnClickListener() {
               @Override
                public void onClick(DialogInterface arg0, int arg1) {
                 cancelUpload();
               }
            });
            if (!mProgressDialog.isShowing()) {
                mProgressDialog.show();
            }
        } else {
            if (!mProgressDialog.isShowing()) {
                mProgressDialog.show();
            }
            mProgressDialog.setProgress(progress);
        }
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case REQUEST_SELECT_SAIUN_LOCAL_FILE:
                    Uri uri3 = data.getData();
                    final String localPath = ImageUtils.getPath(this, uri3);
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                if (ImageUtils.fileIsOutstrip(localPath, ImageUtils.ONE_MB, 500)) {
                                    try {
                                        operation = mMcloudFileApi.putFile(localPath, "",
                                                TransOper.NEW);
                                        Log.i("RCS_UI",
                                                "McloudFileApi ="
                                                        + RcsApiManager.getMcloudFileApi());
                                    } catch (FileSuffixException e) {
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                Toast.makeText(
                                                        RcsShareFileActivity.this,
                                                        mContext.getString(R.string.rcs_saiun_file_upload_legal_file),
                                                        Toast.LENGTH_SHORT).show();
                                            }
                                        });
                                    }
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            showProgressDialog(0);
                                        }
                                    });
                                } else {
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            Toast.makeText(
                                                    RcsShareFileActivity.this,
                                                    mContext.getString(R.string.rcs_saiun_file_upload_legal_file),
                                                    Toast.LENGTH_SHORT).show();
                                        }
                                    });
                                }
                            } catch (ServiceDisconnectedException e) {
                                e.printStackTrace();
                            }
                        }
                    }).start();
                    break;
            }
        }
    }

    private void cancelUpload() {
        if (operation != null) {
            try {
                operation.pause();
            } catch (Exception e) {
                e.printStackTrace();
            }
            Toast.makeText(
                    RcsShareFileActivity.this,
                    mContext.getString(R.string.pause_uploaded),
                    Toast.LENGTH_SHORT).show();
            mProgressDialog.dismiss();
            Log.i("RCS_UI", "CANCEL UPLOAD");
        }
        Log.i("RCS_UI", "CANCEL UPLOAD BUT OPERATION == NULL");
    }

    private void uploadSucess() {
        operation = null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if(receiver != null){
            unregisterReceiver(receiver);
            }
    }

}
