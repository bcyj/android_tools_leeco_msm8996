/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

package com.android.backup;

import android.R.string;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ExpandableListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.TextView;

public class PinnedHeaderExpandableListView extends ExpandableListView implements OnScrollListener {
    private static final String TAG = "PinnedHeaderExpandableListView";

    public interface UpdateHeadViewListener {

        public View getHeader();

        public void updateHeader(View headerView, int firstVisibleGroupPos);
    }

    private View mHeader;
    private int mHeaderViewWidth;
    private int mHeaderViewHeight;

    private TextView mHeaderText;
    private int mHeaderTextViewWidth;
    private String mHeaderTextViewContent;

    private View mFocusView;

    private OnScrollListener mScrollListener;
    private UpdateHeadViewListener mHeaderUpdateListener;

    private boolean mIsDown = false;
    protected boolean mIsHeaderGroupClickable = true;


    public PinnedHeaderExpandableListView(Context context) {
        super(context);
        initView();
    }

    public PinnedHeaderExpandableListView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public PinnedHeaderExpandableListView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView();
    }

    private void initView() {
        setFadingEdgeLength(0);
        setOnScrollListener(this);
    }

    @Override
    public void setOnScrollListener(OnScrollListener l) {
        if (l != this) {
            mScrollListener = l;
        } else {
            mScrollListener = null;
        }
        super.setOnScrollListener(this);
    }

    public void setOnGroupClickListener(OnGroupClickListener onGroupClickListener,
            boolean isHeaderGroupClickable) {
        mIsHeaderGroupClickable = isHeaderGroupClickable;
        super.setOnGroupClickListener(onGroupClickListener);
    }

    public void setOnHeaderUpdateListener(UpdateHeadViewListener listener) {
        mHeaderUpdateListener = listener;
        if (listener == null) {
            mHeader = null;
            mHeaderText = null;
            mHeaderViewWidth = mHeaderViewHeight = 0;
            mHeaderTextViewWidth = 0;
            return;
        }
        mHeader = listener.getHeader();
        mHeaderText = (TextView)mHeader.findViewById(R.id.module_name);
        mHeaderTextViewContent = null;
        int firstVisiblePos = getFirstVisiblePosition();
        int firstVisibleGroupPos = getPackedPositionGroup(getExpandableListPosition(firstVisiblePos));
        listener.updateHeader(mHeader, firstVisibleGroupPos);
        requestLayout();
        postInvalidate();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        if (mHeader == null) {
            return;
        }
        measureChild(mHeader, widthMeasureSpec, heightMeasureSpec);
        mHeaderViewWidth = mHeader.getMeasuredWidth();
        mHeaderViewHeight = mHeader.getMeasuredHeight();
        Log.i(TAG,"onMeasure mHeaderWidth: " + mHeaderViewWidth);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        if (mHeader == null) {
            return;
        }
        int delta = mHeader.getTop();
        mHeader.layout(0, delta, mHeaderViewWidth, mHeaderViewHeight + delta);
    }

    @Override
    protected void dispatchDraw(Canvas canvas) {
        super.dispatchDraw(canvas);
        if (mHeader != null) {
            if (!mHeaderText.getText().equals(mHeaderTextViewContent)) {
                if (mHeaderTextViewWidth != mHeaderText.getWidth()) {
                    drawChild(canvas, mHeader, getDrawingTime());
                    mHeaderTextViewContent = mHeaderText.getText().toString();
                    mHeaderTextViewWidth = mHeaderText.getWidth();
                    Log.i(TAG,"dispatchDraw new group header !");
                } else {
                    // do nothing.
                    Log.i(TAG,"dispatchDraw ignore !");
                }
            } else {
                drawChild(canvas, mHeader, getDrawingTime());
                Log.i(TAG,"dispatchDraw normal group header !");
            }
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        int x = (int) ev.getX();
        int y = (int) ev.getY();
        int pos = pointToPosition(x, y);
        if (mHeader != null && y >= mHeader.getTop() && y <= mHeader.getBottom()) {
            if (ev.getAction() == MotionEvent.ACTION_DOWN) {
                mFocusView = getFocusView(mHeader, x, y);
                mIsDown = true;
            } else if (ev.getAction() == MotionEvent.ACTION_UP) {
                View focusView = getFocusView(mHeader, x, y);
                if (focusView == mFocusView && mFocusView.isClickable()) {
                    mFocusView.performClick();
                    invalidate(new Rect(0, 0, mHeaderViewWidth, mHeaderViewHeight));
                } else if (mIsHeaderGroupClickable){
                    int groupPosition = getPackedPositionGroup(getExpandableListPosition(pos));
                    if (groupPosition != INVALID_POSITION && mIsDown) {
                        if (isGroupExpanded(groupPosition)) {
                            collapseGroup(groupPosition);
                        } else {
                            expandGroup(groupPosition);
                        }
                    }
                }
                mIsDown = false;
            }
            return true;
        }

        return super.dispatchTouchEvent(ev);
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (mScrollListener != null) {
            mScrollListener.onScrollStateChanged(view, scrollState);
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem,
            int visibleItemCount, int totalItemCount) {
        if (totalItemCount > 0) {
            refreshHeaderView();
        }
        if (mScrollListener != null) {
            mScrollListener.onScroll(view, firstVisibleItem, visibleItemCount, totalItemCount);
        }
    }

    private View getFocusView(View view, int x, int y) {
        if (!(view instanceof ViewGroup)) {
            return view;
        }

        ViewGroup parent = (ViewGroup)view;
        int childrenCount = parent.getChildCount();
        final boolean customOrder = isChildrenDrawingOrderEnabled();
        View focus = null;
        for (int i = childrenCount - 1; i >= 0; i--) {
            final int childIndex = customOrder ? getChildDrawingOrder(childrenCount, i) : i;
            final View child = parent.getChildAt(childIndex);
            if (isFocusInView(child, x, y)) {
                focus = child;
                if (child.isClickable()) {
                    return focus;
                } else {
                    return getFocusView(focus, x, y);
                }
            }
        }

        focus = parent;
        return focus;
    }

    private boolean isFocusInView(View view, int x, int y) {
        if (y >= view.getTop() && y <= view.getBottom()
                && x >= view.getLeft() && x <= view.getRight()) {
            return true;
        }
        return false;
    }

    public void requestRefreshHeaderView() {
        refreshHeaderView();
        invalidate(new Rect(0, 0, mHeaderViewWidth, mHeaderViewHeight));
    }

    protected void refreshHeaderView() {
        if (mHeader == null) {
            return;
        }
        int firstVisiblePos = getFirstVisiblePosition();
        int pos = firstVisiblePos + 1;
        int firstVisibleGroupPos = getPackedPositionGroup(getExpandableListPosition(firstVisiblePos));
        int group = getPackedPositionGroup(getExpandableListPosition(pos));

        if (group == firstVisibleGroupPos + 1) {
            View view = getChildAt(1);
            if (view == null) {
                Log.w(TAG, "Warning : refreshHeader getChildAt(1)=null");
                return;
            }
            if (view.getTop() <= mHeaderViewHeight) {
                int delta = mHeaderViewHeight - view.getTop();
                mHeader.layout(0, -delta, mHeaderViewWidth, mHeaderViewHeight - delta);
            } else {
                //TODO : note it, when cause bug, remove it
                mHeader.layout(0, 0, mHeaderViewWidth, mHeaderViewHeight);
            }
        } else {
            mHeader.layout(0, 0, mHeaderViewWidth, mHeaderViewHeight);
        }

        if (mHeaderUpdateListener != null) {
            mHeaderUpdateListener.updateHeader(mHeader, firstVisibleGroupPos);
        }
    }

}