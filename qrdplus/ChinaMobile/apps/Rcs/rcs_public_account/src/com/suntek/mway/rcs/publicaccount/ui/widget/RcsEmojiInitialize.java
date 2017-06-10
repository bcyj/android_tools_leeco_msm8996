/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmojiPackageBO;
import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmoticonBO;
import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmoticonConstant;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;
import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.view.ViewStub;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import java.util.ArrayList;
import java.util.List;

public class RcsEmojiInitialize {

    private Context mContext;

    private ViewStub mViewStub;

    private View mEmojiView = null;

    private GridView mEmojiGridView;

    private GirdViewAdapter mGirdViewAdapter;

    private LinearLayout mLinearLayout;

    private ImageButton mDeleteBtn;

    private String mSelectPackageId = "";

    private String mDefaultPackageId = "-1";

    private ArrayList<EmojiPackageBO> mEmojiPackages = new ArrayList<EmojiPackageBO>();

    private ViewOnClickListener mViewOnClickListener;

    private LinearLayout.LayoutParams mLayoutParams;

    private ArrayList<View> packageListButton = new ArrayList<View>();

    public interface ViewOnClickListener {

        public void viewOpenOrCloseListener(boolean isOpen);

        public void emojiSelectListener(EmoticonBO emojiObject);

        public void faceTextSelectListener(String faceText);

        public void onEmojiDeleteListener();

        public void addEmojiPackageListener();
    }

    public RcsEmojiInitialize(Context context, ViewStub viewStub,
            ViewOnClickListener viewOnClickListener) {
        this.mContext = context;
        this.mViewStub = viewStub;
        this.mViewOnClickListener = viewOnClickListener;
        mLayoutParams = new LinearLayout.LayoutParams(CommonUtil.dip2px(mContext, 45),
                LinearLayout.LayoutParams.MATCH_PARENT);
        mLayoutParams.leftMargin = CommonUtil.dip2px(mContext, 1);
        mSelectPackageId = mDefaultPackageId;
    }

    public void closeOrOpenView() {
        if (mEmojiView == null) {
            CommonUtil.closeKB((Activity)mContext);
            initEmojiView();
            mViewOnClickListener.viewOpenOrCloseListener(true);
            return;
        }
        if (mEmojiView != null && mEmojiView.getVisibility() == View.GONE) {
            CommonUtil.closeKB((Activity)mContext);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    mEmojiView.setVisibility(View.VISIBLE);
                    mViewOnClickListener.viewOpenOrCloseListener(true);
                }
            }, 200);
        } else {
            mEmojiView.setVisibility(View.GONE);
            CommonUtil.openKB(mContext);
            mViewOnClickListener.viewOpenOrCloseListener(false);
        }
    }

    public void closeViewAndKB() {
        mEmojiView.setVisibility(View.GONE);
        mViewOnClickListener.viewOpenOrCloseListener(false);
    }

    public void refreshData(){
        new LoadSessionTask().execute();
    }

    private void initEmojiView() {
        mEmojiView = mViewStub.inflate();
        mEmojiGridView = (GridView)mEmojiView.findViewById(R.id.emoji_grid_view);
        mLinearLayout = (LinearLayout)mEmojiView.findViewById(R.id.content_linear_layout);
        mDeleteBtn = (ImageButton)mEmojiView.findViewById(R.id.delete_emoji_btn);
        mDeleteBtn.setVisibility(View.GONE);
        mEmojiView.findViewById(R.id.add_emoji_btn).setOnClickListener(mClickListener);
        mDeleteBtn.setOnClickListener(mClickListener);
        mGirdViewAdapter = new GirdViewAdapter(mContext, mViewOnClickListener);
        mEmojiGridView.setAdapter(mGirdViewAdapter);
        mDeleteBtn.setVisibility(View.GONE);
        new LoadSessionTask().execute();
    }

    class LoadSessionTask extends AsyncTask<Void, Void, List<EmojiPackageBO>> {
        @Override
        protected List<EmojiPackageBO> doInBackground(Void... params) {
            List<EmojiPackageBO> packageList = new ArrayList<EmojiPackageBO>();
            List<EmojiPackageBO> list = getStorePackageList();
            if (list != null) {
                packageList.addAll(list);
            }
            return packageList;
        }

        @Override
        protected void onPostExecute(List<EmojiPackageBO> result) {
            super.onPostExecute(result);
            if(mEmojiPackages.size() > 0 
                    && mEmojiPackages.size() == result.size()){
                return;
            }
            mEmojiPackages.clear();
            mEmojiPackages.addAll(result);
            initPackageView(result);
            setImageButtonCheck(mSelectPackageId);
            mGirdViewAdapter.setEmojiData(mSelectPackageId);
        }

        private ArrayList<EmojiPackageBO> getStorePackageList() {
            ArrayList<EmojiPackageBO> storelist = new ArrayList<EmojiPackageBO>();
            try {
                List<EmojiPackageBO> list = RcsApiManager.getEmoticonApi().queryEmojiPackages();
                if (list != null && list.size() > 0) {
                    storelist.addAll(list);
                }
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            return storelist;
        }
    }

    private void initPackageView(List<EmojiPackageBO> packageList) {
        mLinearLayout.removeAllViews();
        packageListButton.clear();
        TextView textView = createTextView();
        mLinearLayout.addView(textView);
        packageListButton.add(textView);
        for (int i = 0; i < packageList.size(); i++) {
            EmojiPackageBO emojiPackageBO = packageList.get(i);
            ImageButton imageButton = createImageView(emojiPackageBO);
            mLinearLayout.addView(imageButton);
            packageListButton.add(imageButton);
        }
    }

    private TextView createTextView() {
        TextView textView = new TextView(mContext);
        textView.setLayoutParams(mLayoutParams);
        textView.setPadding(2, 2, 2, 2);
        textView.setTag(mDefaultPackageId);
        textView.setTextSize(25);
        String text = new String(new int[] {
            0x1f601
        }, 0, 1);
        textView.setGravity(Gravity.CENTER);
        textView.setText(text);
        textView.setOnClickListener(mImageOnClickListener);
        return textView;
    }

    private ImageButton createImageView(EmojiPackageBO emojiPackageBO) {
        ImageButton imageButton = new ImageButton(mContext);
        imageButton.setLayoutParams(mLayoutParams);
        imageButton.setScaleType(ScaleType.CENTER_INSIDE);
        imageButton.setPadding(2, 2, 2, 2);
        RcsEmojiStoreUtil.getInstance().loadImageAsynById(imageButton,
                emojiPackageBO.getPackageId(), RcsEmojiStoreUtil.EMO_PACKAGE_FILE);
        imageButton.setTag(emojiPackageBO.getPackageId());
        imageButton.setOnClickListener(mImageOnClickListener);
        return imageButton;
    }

    private OnClickListener mImageOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View view) {
            String packageId = (String)view.getTag();
            if (TextUtils.isEmpty(packageId))
                return;
            if (mSelectPackageId.equals(packageId))
                return;
            mSelectPackageId = packageId;
            setImageButtonCheck(mSelectPackageId);
            mGirdViewAdapter.setEmojiData(mSelectPackageId);
        }
    };

    private void setImageButtonCheck(String checkId) {
        if (checkId.equals(mDefaultPackageId)) {
            mDeleteBtn.setVisibility(View.VISIBLE);
            mEmojiGridView.setNumColumns(7);
            mGirdViewAdapter.setItemHeight(CommonUtil.dip2px(mContext, 45));
        } else {
            mDeleteBtn.setVisibility(View.GONE);
            mEmojiGridView.setNumColumns(4);
            mGirdViewAdapter.setItemHeight(CommonUtil.dip2px(mContext, 80));
        }
        for (View view : packageListButton) {
            String packageId = (String)view.getTag();
            if (!packageId.equals(checkId))
                view.setBackgroundResource(R.color.gray5);
            else
                view.setBackgroundResource(R.color.white);
        }
    }

    private OnClickListener mClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.delete_emoji_btn:
                    mViewOnClickListener.onEmojiDeleteListener();
                    break;
                case R.id.add_emoji_btn:
                    mViewOnClickListener.addEmojiPackageListener();
                    break;
                default:
                    break;
            }
        }
    };

    public class GirdViewAdapter extends BaseAdapter {

        private ArrayList<EmoticonBO> mEmojiObjects = new ArrayList<EmoticonBO>();

        private Context mContext;

        private int mItemHeight;

        private String mPackageId = "";

        private ViewOnClickListener mViewOnClickListener;

        public GirdViewAdapter(Context context, ViewOnClickListener viewOnClickListener) {
            this.mContext = context;
            this.mViewOnClickListener = viewOnClickListener;
        }

        private void setItemHeight(int height) {
            this.mItemHeight = height;
        }

        public void setEmojiData(String packageId) {
            this.mPackageId = packageId;
            if (mPackageId.equals(mDefaultPackageId)) {
                this.mEmojiObjects.clear();
                this.notifyDataSetChanged();
                return;
            }
            try {
                List<EmoticonBO> list = RcsApiManager.getEmoticonApi().queryEmoticons(packageId);
                if (list != null && list.size() > 0) {
                    this.mEmojiObjects.clear();
                    this.mEmojiObjects.addAll(list);
                    this.notifyDataSetChanged();
                }
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
        }

        @Override
        public int getCount() {
            if (mPackageId.equals(mDefaultPackageId)) {
                return mFaceTexts.length;
            }
            return mEmojiObjects.size();
        }

        @Override
        public Object getItem(int position) {
            if (mPackageId.equals(mDefaultPackageId)) {
                return mFaceTexts[position];
            }
            return mEmojiObjects.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if (convertView == null) {
                convertView = LayoutInflater.from(mContext).inflate(
                        R.layout.rcs_emoji_grid_view_item, null);
                holder = new ViewHolder(convertView);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder)convertView.getTag();
            }
            holder.setItemHeight(mItemHeight);
            if (mPackageId.equals(mDefaultPackageId)) {
                int faceInt = (Integer)getItem(position);
                holder.title.setVisibility(View.GONE);
                holder.icon.setVisibility(View.GONE);
                holder.textFace.setVisibility(View.VISIBLE);
                String faceText = new String(new int[] {
                        faceInt }, 0, 1);
                holder.textFace.setText(faceText);
                holder.mItemView.setTag(faceText);
                holder.mItemView.setOnClickListener(mClickListener);
            } else {
                holder.textFace.setVisibility(View.GONE);
                holder.icon.setVisibility(View.VISIBLE);
                holder.title.setVisibility(View.VISIBLE);
                EmoticonBO bean = (EmoticonBO)getItem(position);
                holder.title.setText(bean.getEmoticonName());
                RcsEmojiStoreUtil.getInstance().loadImageAsynById(holder.icon,
                        bean.getEmoticonId(), RcsEmojiStoreUtil.EMO_STATIC_FILE);
                holder.mItemView.setTag(bean);
                holder.mItemView.setOnClickListener(mClickListener);
                holder.mItemView.setOnLongClickListener(onLongClickListener);
            }
            return convertView;
        }

        private OnLongClickListener onLongClickListener = new OnLongClickListener() {
            @Override
            public boolean onLongClick(View arg0) {
                try {
                    EmoticonBO bean = (EmoticonBO)arg0.getTag();
                    byte[] data = RcsApiManager.getEmoticonApi().decrypt2Bytes(
                            bean.getEmoticonId(), EmoticonConstant.EMO_DYNAMIC_FILE);
                    CommonUtil.openPopupWindow(mContext, arg0, data);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
                return false;
            }
        };

        private OnClickListener mClickListener = new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mPackageId.equals(mDefaultPackageId)) {
                    String faceText = (String)view.getTag();
                    mViewOnClickListener.faceTextSelectListener(faceText);
                } else {
                    EmoticonBO bean = (EmoticonBO)view.getTag();
                    mViewOnClickListener.emojiSelectListener(bean);
                }
            }
        };

        private final int[] mFaceTexts = new int[] {
                0x1f600, 0x1f601, 0x1f602, 0x1f603, 0x1f604, 0x1f605, 0x1f606, 0x1f607, 0x1f608,
                0x1f609, 0x1f60A, 0x1f60B, 0x1f60C, 0x1f60D, 0x1f60E, 0x1f60F, 0x1f610, 0x1f611,
                0x1f612, 0x1f613, 0x1f614, 0x1f615, 0x1f616, 0x1f617, 0x1f618, 0x1f619, 0x1f61A,
                0x1f61B, 0x1f61C, 0x1f61D, 0x1f61F, 0x1f620, 0x1f621, 0x1f622, 0x1f623, 0x1f624,
                0x1f625, 0x1f626, 0x1f627, 0x1f628, 0x1f629, 0x1f62A, 0x1f62B, 0x1f62C, 0x1f62D,
                0x1f62F, 0x1f630, 0x1f631, 0x1f632, 0x1f633, 0x1f634, 0x1f635, 0x1f636, 0x1f637,
                0x1f638, 0x1f639, 0x1f63A, 0x1f63B, 0x1f63C, 0x1f63D, 0x1f63F, 0x1f640, 0x1f645,
                0x1f646, 0x1f647, 0x1f648, 0x1f649, 0x1f64A, 0x1f64B, 0x1f64C, 0x1f64D, 0x1f64F
        };

        private class ViewHolder {
            RelativeLayout mItemView;

            TextView title;

            TextView textFace;

            ImageView icon;

            public void setItemHeight(int height) {
                RelativeLayout.LayoutParams param = (RelativeLayout.LayoutParams)this.mItemView
                        .getLayoutParams();
                param.height = height;
            }

            public ViewHolder(View convertView) {
                this.title = (TextView)convertView.findViewById(R.id.title);
                this.icon = (ImageView)convertView.findViewById(R.id.icon);
                this.textFace = (TextView)convertView.findViewById(R.id.text_face);
                this.mItemView = (RelativeLayout)convertView.findViewById(R.id.item);
                this.mItemView.setBackgroundResource(R.drawable.rcs_public_account_btn_bg);
            }
        }
    }

}
