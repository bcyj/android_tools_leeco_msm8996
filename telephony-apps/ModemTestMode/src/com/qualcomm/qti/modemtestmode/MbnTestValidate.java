/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.io.File;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.channels.FileChannel;

import android.app.Activity;
import android.app.Dialog;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.widget.ProgressBar;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.res.XmlResourceParser;
import android.content.Context;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Gravity;
import android.widget.Toast;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ExpandableListView;
import android.widget.BaseExpandableListAdapter;
import android.graphics.drawable.Drawable;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import android.telephony.TelephonyManager;
import org.xmlpull.v1.XmlPullParser;

public class MbnTestValidate extends Activity {
    private final String TAG = "MbnTestValidate";

    private static final int VALIDATION_INIT = 0;
    private static final int VALIDATION_IN_PROGRESS = 1;
    private static final int VALIDATION_DONE = 2;
    private static final int VALIDATION_GET_QV_FAIL = 3;
    private static final int VALIDATION_HANDLE_DIR_FAIL = 4;
    private static final int VALIDATION_FIND_REF_MBN_FAIL = 5;

    private static final int EVENT_QCRIL_HOOK_READY = 1;
    private static final int EVENT_START_VALIDATE = 2;
    private static final int EVENT_HANDLE_REFMBN_FAIL = 3;

    private static final int FAIL_TO_GET_QC_VER = 1;
    private static final int FAIL_TO_HANDLE_DIR = 2;
    private static final int FAIL_TO_FIND_REF_MBN = 3;


    private MbnMetaInfo mMbnMetaInfo;

    private String mRefMbnPath;
    private String mRefMbnConfig;
    private Context mContext;
    private boolean mValidationShowAll = false;

    private int[] mValidationStatus = null;
    private ValiExpaListAdapter[] mListExpListAdapter = null;
    private ExpandableListView mExpListView = null;
    private List<String>[] mListGroup = null;
    private HashMap<String, List<String>>[] mListChild = null;
    private Button mStartValidateButon[] = null;
    private Drawable mDrawable[] = null;
    private int mCurValidateSub = 0;
    private int mPhoneCount = 0;
    private HashMap<String, String> mRulesEngineMap = null;

    private BroadcastReceiver sValidationReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            log("Received:" + action);
            if (MbnTestUtils.PDC_DATA_ACTION.equals(action)) {
                byte[] result = intent.getByteArrayExtra(MbnTestUtils.PDC_ACTIVATED);
                int sub = intent.getIntExtra(MbnTestUtils.SUB_ID, MbnTestUtils.DEFAULT_SUB);
                ByteBuffer payload = ByteBuffer.wrap(result);
                payload.order(ByteOrder.nativeOrder());
                int error = payload.get();
                log("Sub:" + sub + " activated:" + new String(result) + " error code:" + error);
                if (error == MbnTestUtils.LOAD_MBN_SUCCESS) {
                    Message msg = mHandler.obtainMessage(EVENT_START_VALIDATE);
                    msg.arg1 = mCurValidateSub;
                    msg.sendToTarget();
                } else {
                    //TODO need add a status for mValidationStatus
                    setOtherSubsClickable(mCurValidateSub);
                    mValidationStatus[mCurValidateSub] = VALIDATION_DONE;
                }
            } else if (MbnTestUtils.PDC_CONFIGS_VALIDATION_ACTION.equals(action)) {
                Bundle data = intent.getExtras();
                int sub = data.getInt("sub");
                int index = data.getInt("index");
                String nvItemInfo = data.getString("nv_item");
                String refValue= data.getString("ref_value");
                String curValue= data.getString("cur_value");
                log("----  index:" + index);
                if (index == -1) {
                    mValidationStatus[mCurValidateSub] = VALIDATION_DONE;
                    MbnTestUtils.showToast(mContext, String.format(mContext.getString(
                            R.string.sub_validation_done), mCurValidateSub+1));
                    setOtherSubsClickable(mCurValidateSub);
                } else {
                    addNVData(mCurValidateSub, nvItemInfo, refValue, curValue);
                }
            }
        }
    };

    // update UI
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            log("sub: "+msg.arg1);
            switch (msg.what) {
            case EVENT_QCRIL_HOOK_READY:
                MbnAppGlobals.getInstance().unregisterQcRilHookReady(mHandler);
                setActivityView();
                break;

            case EVENT_START_VALIDATE:
                log("EVENT_START_VALIDATE");
                MbnAppGlobals.getInstance().validateConfig(mRefMbnConfig, mCurValidateSub);
                break;

            case EVENT_HANDLE_REFMBN_FAIL:
                log("EVENT_HANDLE_REFMBN_FAIL error:" + msg.arg2);
                switch(msg.arg2) {
                    case FAIL_TO_GET_QC_VER:
                    MbnTestUtils.showToast(mContext, mContext.getString(
                            R.string.get_activated_qcver_fail));
                    mValidationStatus[msg.arg1] = VALIDATION_GET_QV_FAIL;
                    break;

                    case FAIL_TO_HANDLE_DIR:
                    MbnTestUtils.showToast(mContext, String.format(mContext.getString(
                            R.string.fail_to_handle_dir), MbnTestUtils.getGoldenMbnPath()));
                    mValidationStatus[msg.arg1] = VALIDATION_HANDLE_DIR_FAIL;
                    break;

                    case FAIL_TO_FIND_REF_MBN:
                    MbnTestUtils.showToast(mContext, mContext.getString(
                            R.string.fail_to_find_godlen_mbn));
                    mValidationStatus[msg.arg1] = VALIDATION_FIND_REF_MBN_FAIL;
                    break;

                    default:
                    //default to VALIDATION_GET_QV_FAIL
                    mValidationStatus[msg.arg1] = VALIDATION_GET_QV_FAIL;
                    break;
                }

                break;

            default:
                log("wrong event");
                break;
            }
        }
    };

    private void getRulesEngineMap() {
        mRulesEngineMap = new HashMap<String, String>();
        XmlResourceParser parser = mContext.getResources().getXml(R.xml.rules_engine);
        String key = null, value = null;
        try {
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_DOCUMENT) {
                    log("begin to load rules engine");
                } else if (eventType == XmlPullParser.START_TAG) {
                    if (parser.getName().equals("entry")) {
                        key = parser.getAttributeValue(null, "key");
                        if (key == null) {
                            log("key can't be null");
                            return;
                        }
                    }
                } else if (eventType == XmlPullParser.TEXT) {
                    if (key != null) {
                        value = parser.getText();
                    }
                } else if (eventType == XmlPullParser.END_TAG) {
                    if (parser.getName().equals("entry")) {
                        //log(key + "   " + value);
                        mRulesEngineMap.put(key, value);
                        value = null;
                        key = null;
                    }
                }
                eventType = parser.next();
            }
        } catch (Exception e) {
            log("Got exception while loading rules engine " + e);
        } finally {
            parser.close();
        }
    }

    private void setActivityView() {

        if (!handleMbnIdMetaInfo()) {
            // TODO need show alert.
            finish();
            return;
        }

        mValidationShowAll = MbnTestUtils.mbnValidationShowAll();

        mPhoneCount = TelephonyManager.getDefault().getPhoneCount();
        getRulesEngineMap();

        mStartValidateButon = new Button[mPhoneCount];
        mDrawable = new Drawable[mPhoneCount];
        mValidationStatus = new int[mPhoneCount];
        for (int i = 0; i < mPhoneCount; i++) {
            mValidationStatus[i] = VALIDATION_INIT;
            int viewId = R.id.validate_sub1;
            if (i > 0) {
                viewId = R.id.validate_sub2;
            }
            mStartValidateButon[i] = (Button) findViewById(viewId);
            mDrawable[i] = mStartValidateButon[i].getBackground();
            mStartValidateButon[i].setOnClickListener(
                    new Button.OnClickListener() {
                        public void onClick(View v) {
                            switch (v.getId()) {
                            case R.id.validate_sub1:
                                //mStartValidateButon[1].setClickable(false);
                                //mStartValidateButon[0].setBackgroundResource(
                                //        android.R.drawable.btn_default);
                                mStartValidateButon[0].setBackgroundDrawable(mDrawable[0]);
                                if (mPhoneCount > 1) {
                                    //mStartValidateButon[1].setBackgroundColor(0);
                                    mStartValidateButon[1].setBackgroundDrawable(null);
                                }
                                startMbnValidate(0);
                                break;
                            case R.id.validate_sub2:
                                //mStartValidateButon[0].setClickable(false);
                                //mStartValidateButon[1].setBackgroundResource(
                                //        android.R.drawable.btn_default);
                                mStartValidateButon[1].setBackgroundDrawable(mDrawable[1]);
                                //mStartValidateButon[0].setBackgroundColor(0);
                                mStartValidateButon[0].setBackgroundDrawable(null);
                                startMbnValidate(1);
                                break;
                            }
                        }
                    });
            mStartValidateButon[i].setVisibility(View.VISIBLE);
        }
        setExpandableListView();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.mbn_validate);
        mContext = this;
        MbnAppGlobals.getInstance().registerQcRilHookReady(mHandler,
                EVENT_QCRIL_HOOK_READY, null);
    }

    @Override
    protected void onResume() {
        super.onResume();
        IntentFilter filter = new IntentFilter();
        filter.addAction(MbnTestUtils.PDC_DATA_ACTION);
        filter.addAction(MbnTestUtils.PDC_CONFIGS_VALIDATION_ACTION);
        registerReceiver(sValidationReceiver, filter);
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(sValidationReceiver);
    }

    private void setExpandableListView() {
        mExpListView = (ExpandableListView) findViewById(R.id.nv_diff_data);
        mListExpListAdapter = new ValiExpaListAdapter[mPhoneCount];
        mListGroup = new ArrayList[mPhoneCount];
        mListChild = new HashMap[mPhoneCount];
        for (int subId = 0; subId < mPhoneCount; subId++) {
            mListGroup[subId] = new ArrayList<String>();
            mListChild[subId] = new HashMap<String, List<String>>();
            mListExpListAdapter[subId] = new ValiExpaListAdapter(this, this, subId);
        }
    }

    public void onDestroy() {
        super.onDestroy();
    }

    private boolean otherSubValiateCheck(int sub, int event) {
        for (int i = 0; i < mPhoneCount; i++) {
            if (i != sub && mValidationStatus[i] == event) {
                log("mStatus[" + i + "]=" + mValidationStatus[i]);
                return true;
            }
        }
        return false;
    }

    private void setOtherSubsClickable(int sub) {
        // TODO need?
      /*for (int i = 0; i < mPhoneCount; i++) {
            if (i != sub) {
                mStartValidateButon[i].setClickable(true);
            }
        }*/
        return;
    }

    private void startMbnValidate(int sub) {
        log("startMbnValidate sub " + sub);
        if (otherSubValiateCheck(sub, VALIDATION_IN_PROGRESS)) {
            MbnTestUtils.showToast(this, mContext.getString(R.string.other_sub_is_validating));
        } else if (mValidationStatus[sub] == VALIDATION_INIT) {
            MbnTestUtils.showToast(this, String.format(mContext.getString(
                    R.string.begin_to_validate), sub+1));
            mValidationStatus[sub] = VALIDATION_IN_PROGRESS;
            mExpListView.setAdapter(mListExpListAdapter[sub]);

            if (otherSubValiateCheck(sub, VALIDATION_GET_QV_FAIL)) {
                mValidationStatus[sub] = VALIDATION_GET_QV_FAIL;
                MbnTestUtils.showToast(this, mContext.getString(R.string.get_qcver_fail));
                setOtherSubsClickable(sub);
                return;
            }

            if (otherSubValiateCheck(sub, VALIDATION_HANDLE_DIR_FAIL)) {
                mValidationStatus[sub] = VALIDATION_HANDLE_DIR_FAIL;
                MbnTestUtils.showToast(this, String.format(mContext.getString(
                        R.string.fail_to_handle_dir), MbnTestUtils.getGoldenMbnPath()));
                setOtherSubsClickable(sub);
                return;
            }

            if (otherSubValiateCheck(sub, VALIDATION_FIND_REF_MBN_FAIL)) {
                mValidationStatus[sub] = VALIDATION_FIND_REF_MBN_FAIL;
                MbnTestUtils.showToast(mContext, mContext.getString(
                        R.string.fail_to_find_godlen_mbn));
                setOtherSubsClickable(sub);
                return;
            }

            if (otherSubValiateCheck(sub, VALIDATION_DONE)) {
                Message msg = mHandler.obtainMessage(EVENT_START_VALIDATE);
                msg.arg1 = sub;
                mCurValidateSub = sub;
                mHandler.sendMessage(msg);
            } else {
                new Thread(new RefMbnLoadingThread(sub)).start();
            }
        } else if(mValidationStatus[sub] == VALIDATION_IN_PROGRESS) {
            MbnTestUtils.showToast(this, String.format(mContext.getString(
                    R.string.sub_is_validating), sub+1));
        } else if(mValidationStatus[sub] == VALIDATION_DONE) {
            mExpListView.setAdapter(mListExpListAdapter[sub]);
        } else if (mValidationStatus[sub] == VALIDATION_GET_QV_FAIL) {
            MbnTestUtils.showToast(this, mContext.getString(R.string.get_qcver_fail));
        } else if (mValidationStatus[sub] == VALIDATION_HANDLE_DIR_FAIL) {
            MbnTestUtils.showToast(this, String.format(mContext.getString(
                    R.string.fail_to_handle_dir), MbnTestUtils.getGoldenMbnPath()));
        } else if (mValidationStatus[sub] == VALIDATION_FIND_REF_MBN_FAIL) {
            MbnTestUtils.showToast(mContext, mContext.getString(
                    R.string.fail_to_find_godlen_mbn));
        } else {
            log("Wrong status");
        }
        setOtherSubsClickable(sub);
    }

    private boolean handleMbnIdMetaInfo() {
        String mbnId = MbnAppGlobals.getInstance().getMbnConfig(0);
        String metaInfo = MbnAppGlobals.getInstance().getMetaInfoForConfig(mbnId);
        mMbnMetaInfo = new MbnMetaInfo(MbnTestValidate.this, mbnId, metaInfo);
        log("Cur Mbn: " + mMbnMetaInfo);
        //TODO need for all activated mbn validation?
        if (mbnId == null || metaInfo == null) {
            return false;
        }
        return true;
    }

    private void addNVData(int sub, String group, String ref, String cur) {
        String prefix = "";
        if (mRulesEngineMap.containsKey(group.trim())) {
            String[] tmp = mRulesEngineMap.get(group.trim()).split("\\|");
            if (tmp.length == 2) {
                String[] options = tmp[0].split(",");
                int index = Integer.parseInt(options[0]);
                if (options.length == 2) {
                    if (sub == 1) {
                        index = Integer.parseInt(options[1]);
                    }
                }
                switch (index) {
                    case 1:
                        prefix = tmp[1] + " " + group + " " +
                                mContext.getString(R.string.rule1) + "\n";
                        break;
                    case 2:
                        prefix = tmp[1] + " " + group + " " +
                                mContext.getString(R.string.rule2) + "\n";
                        break;
                    case 3:
                        prefix = tmp[1] + " " + group + " " +
                                mContext.getString(R.string.rule3) + "\n";
                        break;
                    case 4:
                        prefix = tmp[1] + " " + group + " " +
                                mContext.getString(R.string.rule4) + "\n";
                        break;
                    case 5:
                        prefix = tmp[1] + " " + group + " " +
                                mContext.getString(R.string.rule5) + "\n";
                        break;
                    default:
                        break;
                }
            }
        } else {
            // Just check if need to show all
            if (mValidationShowAll == false) {
                return;
            }
        }

        mListGroup[sub].add(group);
        List<String> list = new ArrayList<String>();
        list.add(prefix + mContext.getString(R.string.ref_value) + ref);
        list.add(mContext.getString(R.string.cur_value)+cur);
        mListChild[sub].put(mListGroup[sub].get(mListGroup[sub].size()-1), list);
        mListExpListAdapter[sub].notifyDataSetChanged();
        //log("" + listGroup[sub] + "  " + listChild[sub] + " " + listGroup[sub]);
    }

    // TODO Don't like this method
    private void reverseDirToLoadRefMbn(File dir) {
        if (mRefMbnPath != null) {
            return;
        }
        try {
            File[] files = dir.listFiles();
            for (File file: files) {
                if (file.isDirectory()) {
                    reverseDirToLoadRefMbn(file);
                } else {
                    if (file.canRead() && file.getName().endsWith(".mbn")) {
                        String origPath = file.getCanonicalPath();
                        String destName = file.getName();
                        // TODO need copy to somewhere
                        if (MbnTestUtils.mbnNeedToGo()) {
                            destName = MbnTestUtils.MBN_TO_GO_DIR+destName;
                            File dest = new File(destName);
                            try {
                                dest.delete();
                                dest.createNewFile();
                                Runtime.getRuntime().exec("chmod 644 " + destName);
                                Runtime.getRuntime().exec("chown radio.radio " + destName);
                            } catch (IOException e) {
                                log("can't create file:" + e);
                                continue;
                            }
                            log("copy " + file.getAbsolutePath() + " to " + dest.getAbsolutePath());
                            if (!MbnFileManager.copyWithChannels(file, dest, true)) {
                                MbnTestUtils.showToast(mContext, mContext.getString(
                                        R.string.fail_to_copy_file));
                                continue;
                            }
                        } else {
                            destName = origPath;
                        }

                        byte[] refQV = MbnAppGlobals.getInstance().getQcVersionOfFile(destName);
                        if (refQV == null || refQV.length != mMbnMetaInfo.getQcVersion().length) {
                            log("Cur QV length:" + mMbnMetaInfo.getQcVersion().length + "Ref QV:" +
                                    refQV + " path:" + destName);
                            continue;
                        }

                        int j = 0;
                        for (j = 0; j < mMbnMetaInfo.getQcVersion().length; j++) {
                            log("index:" + j + " Cur:" + mMbnMetaInfo.getQcVersion()[j] +
                                    " Ref:" + refQV[j]);
                            if (mMbnMetaInfo.getQcVersion()[j] != refQV[j]) {
                                break;
                            }
                        }

                        if (j == refQV.length) {
                            mRefMbnPath = destName;
                            mRefMbnConfig = MbnTestUtils.GOLDEN_MBN_ID_PREFIX+origPath;
                            log("Find Golden Mbn:" + mRefMbnPath);
                            return;
                        }
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public class RefMbnLoadingThread implements Runnable {
        private int mSub;

        public RefMbnLoadingThread(int sub) {
            mSub = sub;
        }

        public void run() {
            // 1. get current mbn qc version
            byte[] qcVersion = MbnAppGlobals.getInstance().getQcVersionOfID(mMbnMetaInfo.getMbnId());
            if (qcVersion == null) {
                log("Fail to get qc version sub" + mSub);
                Message msg = mHandler.obtainMessage(EVENT_HANDLE_REFMBN_FAIL);
                msg.arg1 = mSub;
                msg.arg2 = FAIL_TO_GET_QC_VER;
                msg.sendToTarget();
                return;
            }
            mMbnMetaInfo.setQcVersion(qcVersion);

            // 2. begin to get ref qc version and load, reverse a Golden Dir
            File file = new File(MbnTestUtils.getGoldenMbnPath());
            if (file.exists() && file.canRead()) {
                mRefMbnPath = null;
                reverseDirToLoadRefMbn(file);
                if (mRefMbnPath != null && mRefMbnConfig != null) {
                    // 3. success, need load to modem.
                    MbnAppGlobals.getInstance().setupMbnConfig(mRefMbnPath, mRefMbnConfig, 0);
                    mCurValidateSub = mSub;
                    log("Waiting for validating result ... for sub" + mSub);
                } else {
                    log("Fail to find Golden Mbn for sub" + mSub);
                    Message msg = mHandler.obtainMessage(EVENT_HANDLE_REFMBN_FAIL);
                    msg.arg1 = mSub;
                    msg.arg2 = FAIL_TO_FIND_REF_MBN;
                    msg.sendToTarget();
                }
            } else {
                log("Fail to find handle dir for sub" + mSub);
                Message msg = mHandler.obtainMessage(EVENT_HANDLE_REFMBN_FAIL);
                msg.arg1 = mSub;
                msg.arg2 = FAIL_TO_HANDLE_DIR;
                msg.sendToTarget();
            }
        }
    }

    private class ValiExpaListAdapter extends BaseExpandableListAdapter {
        private Activity mActivity;
        private int mSubId;

        public ValiExpaListAdapter(Context context, Activity activity, int sub) {
            mActivity = activity;
            mSubId = sub;
        }

        public Object getChild(int groupPosition, int childPosition) {
            String tmp = mListGroup[mSubId].get(groupPosition);
            return mListChild[mSubId].get(tmp).get(childPosition);
        }

        public long getChildId(int groupPosition, int childPosition) {
            return childPosition;
        }

        public int getChildrenCount(int groupPosition) {
            String tmp = mListGroup[mSubId].get(groupPosition);
            return mListChild[mSubId].get(tmp).size();
        }

        public View getChildView(int groupPosition, int childPosition,
                boolean isLastChild, View convertView, ViewGroup parent) {
            String childText = (String)getChild(groupPosition, childPosition);
            if (convertView == null) {
                convertView = mActivity.getLayoutInflater().inflate(
                        R.layout.list_child, null);
            }
            TextView tv = (TextView)convertView.findViewById(R.id.list_child);
            tv.setText(childText);
            return convertView;
        }

        public boolean isChildSelectable(int groupPosition, int childPosition) {
            return true;
        }

        public Object getGroup(int groupPosition) {
            return mListGroup[mSubId].get(groupPosition);
        }

        public int getGroupCount() {
            return mListGroup[mSubId].size();
        }

        public long getGroupId(int groupPosition) {
            return groupPosition;
        }

        public View getGroupView(int groupPosition, boolean isExpanded,
                View convertView, ViewGroup parent) {
            String groupTitle = (String) getGroup(groupPosition);
            if (convertView == null) {
                convertView = mActivity.getLayoutInflater().inflate(
                        R.layout.list_group, null);
            }
            TextView tv = (TextView) convertView.findViewById(R.id.list_group);
            tv.setText(groupTitle);
            if (mRulesEngineMap.containsKey(groupTitle.trim())) {
                tv.setTextColor(0xFFEF4136);
            } else {
                tv.setTextColor(0xFFF9F900);
            }
            return convertView;
        }

        public boolean hasStableIds() {
            return false;
        }
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_ " + msg);
    }
}
