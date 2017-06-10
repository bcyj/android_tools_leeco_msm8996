/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import com.suntek.mway.rcs.publicaccount.R;
import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ProgressBar;
import android.widget.AbsListView.OnScrollListener;
import android.widget.ListView;

public class HistoryMsgListView extends ListView implements OnScrollListener {

    private static final int SPACE = 20;

    public static final int PA_REFRESH = 0;

    private static final int PA_REFRESHING = 3;

    private static final int PA_RELEASE = 2;

    private static final int PA_PULL = 1;

    private static final int PA_NONE = 0;

    private int mState;

    private View mHeader;

    private ProgressBar mRefreshing;

    private int mStartY;

    private int mFirstVisibleItem;

    private int mScrollState;

    private int mHeaderContentInitialHeight;

    private int mHeaderContentHeight;

    private boolean mIsRecorded;

    private OnPARefreshListener onRefreshListener;

    private PAListScrollListener mPAListScrollListener;

    public HistoryMsgListView(Context context) {
        super(context);
        initView(context);
    }

    public HistoryMsgListView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView(context);
    }

    public HistoryMsgListView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView(context);
    }

    public void setPARefreshListener(OnPARefreshListener onRefreshListener) {
        this.onRefreshListener = onRefreshListener;
    }

    private void initView(Context context) {
        mHeader = LayoutInflater.from(context).inflate(R.layout.pull_to_refresh_header, null);
        mRefreshing = (ProgressBar)mHeader.findViewById(R.id.refreshing);

        mHeaderContentInitialHeight = mHeader.getPaddingTop();
        measureHistoryView(mHeader);
        mHeaderContentHeight = mHeader.getMeasuredHeight();
        topPadding(-mHeaderContentHeight);
        this.addHeaderView(mHeader);
        this.setOnScrollListener(this);
    }

    public void onPARefresh() {
        if (onRefreshListener != null) {
            onRefreshListener.onPARefresh();
        }
    }

    public void onPARefreshComplete() {
        mState = PA_NONE;
        refreshHistoryHeaderViewByState();
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        this.mFirstVisibleItem = firstVisibleItem;
        if (mPAListScrollListener != null) {
            mPAListScrollListener.onPAScroll(view, firstVisibleItem, visibleItemCount, totalItemCount);
        }
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        this.mScrollState = scrollState;
        if (mPAListScrollListener != null) {
            mPAListScrollListener.onPAScrollStateChanged(view, scrollState);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        switch (ev.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (mFirstVisibleItem == 0) {
                    mIsRecorded = true;
                    mStartY = (int)ev.getY();
                }
                break;
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                if (mState == PA_PULL) {
                    mState = PA_NONE;
                    refreshHistoryHeaderViewByState();
                } else if (mState == PA_RELEASE) {
                    mState = PA_REFRESHING;
                    refreshHistoryHeaderViewByState();
                    onPARefresh();
                }
                mIsRecorded = false;
                break;
            case MotionEvent.ACTION_MOVE:
                whenMove(ev);
                break;
        }
        return super.onTouchEvent(ev);
    }

    private void whenMove(MotionEvent ev) {
        if (!mIsRecorded) {
            return;
        }
        int tmpY = (int)ev.getY();
        int space = tmpY - mStartY;
        int topPadding = space - mHeaderContentHeight;
        switch (mState) {
            case PA_NONE:
                if (space > 0) {
                    mState = PA_PULL;
                    refreshHistoryHeaderViewByState();
                }
                break;
            case PA_PULL:
                topPadding(topPadding);
                if (mScrollState == SCROLL_STATE_TOUCH_SCROLL && space > mHeaderContentHeight + SPACE) {
                    mState = PA_RELEASE;
                    refreshHistoryHeaderViewByState();
                }
                break;
            case PA_RELEASE:
                topPadding(topPadding);
                if (space > 0 && space < mHeaderContentHeight + SPACE) {
                    mState = PA_PULL;
                    refreshHistoryHeaderViewByState();
                } else if (space <= 0) {
                    mState = PA_NONE;
                    refreshHistoryHeaderViewByState();
                }
                break;
        }
    }

    private void topPadding(int topPadding) {
        mHeader.setPadding(mHeader.getPaddingLeft(), topPadding, mHeader.getPaddingRight(),
                mHeader.getPaddingBottom());
        mHeader.invalidate();
    }

    private void refreshHistoryHeaderViewByState() {
        switch (mState) {
            case PA_NONE:
                topPadding(-mHeaderContentHeight);
                mRefreshing.setVisibility(View.GONE);
                break;
            case PA_PULL:
                mRefreshing.setVisibility(View.GONE);
                break;
            case PA_RELEASE:
                mRefreshing.setVisibility(View.GONE);
                break;
            case PA_REFRESHING:
                topPadding(mHeaderContentInitialHeight);
                mRefreshing.setVisibility(View.VISIBLE);
                break;
        }
    }

    public interface PAListScrollListener {
        public void onPAScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
                int totalItemCount);
        public void onPAScrollStateChanged(AbsListView view, int scrollState);
    }

    public void setPAListScrollListener(PAListScrollListener paListScrollListener) {
        this.mPAListScrollListener = paListScrollListener;
    }

    public interface OnPARefreshListener {
        public void onPARefresh();
    }

    private void measureHistoryView(View childView) {
        ViewGroup.LayoutParams params = childView.getLayoutParams();
        if (params == null) {
            params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
        }
        int width = params.width;
        int height = params.height;
        int paChildHeightSpec;
        int paChildWidthSpec = ViewGroup.getChildMeasureSpec(0, 0 + 0, width);
        if (height > 0) {
            paChildHeightSpec = MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY);
        } else {
            paChildHeightSpec = MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED);
        }
        childView.measure(paChildWidthSpec, paChildHeightSpec);
    }
}
