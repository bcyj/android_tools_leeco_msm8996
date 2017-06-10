/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.android.timerswitch;

import java.text.DateFormatSymbols;
import java.util.Calendar;
import java.util.HashSet;

import com.android.timerswitch.provider.DaysOfWeek;
import com.android.timerswitch.provider.TimerSwitch;
import com.android.timerswitch.utils.Log;
import com.android.timerswitch.utils.Utils;
import com.android.timerswitch.widget.TextTime;
import com.android.datetimepicker.time.TimePickerDialog;
import com.android.datetimepicker.time.RadialPickerLayout;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.animation.Animator.AnimatorListener;
import android.app.Fragment;
import android.app.LoaderManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.Loader;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Vibrator;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.text.format.DateFormat;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CursorAdapter;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.ToggleButton;

public class TimerSwitchSettingsFragment extends Fragment implements
        LoaderManager.LoaderCallbacks<Cursor>,
        TimePickerDialog.OnTimeSetListener,
        View.OnTouchListener {

    public TimerSwitchSettingsFragment() {
    }

    private static final String KEY_SWITCH_ON_EBABLER = "timer_switch_on";
    private static final String KEY_SWITCH_OFF_EBABLER = "timer_switch_off";

    private static final float EXPAND_DECELERATION = 1f;
    private static final float COLLAPSE_DECELERATION = 0.7f;
    private static int DAYS_COUNT_WEEK = 7;
    private static final int ANIMATION_DURATION = 300;
    private static final String KEY_EXPANDED_IDS = "expandedIds";
    private static final String KEY_REPEAT_CHECKED_IDS = "repeatCheckedIds";
    private static final String KEY_RINGTONE_TITLE_CACHE = "ringtoneTitleCache";
    private static final String KEY_SELECTED_ALARMS = "selectedAlarms";
    private static final String KEY_PREVIOUS_DAY_MAP = "previousDayMap";
    private static final String KEY_SELECTED_TIMER_SWITCH = "selectedTimerSwitch";

    private SwitchItemAdapter mSwitchAdapter;
    ListView mSwitchList;
    private TimerSwitch mSelectedTimerSwitch;

    private Interpolator mExpandInterpolator;
    private Interpolator mCollapseInterpolator;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("OnCreate");
        getLoaderManager().initLoader(0, null, this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedState) {

        final View v = inflater.inflate(R.layout.timer_switch, container, false);
        long[] expandedIds = null;
        long[] repeatCheckedIds = null;
        long[] selectedAlarms = null;
        Bundle previousDayMap = null;
        if (savedState != null) {
            expandedIds = savedState.getLongArray(KEY_EXPANDED_IDS);
            repeatCheckedIds = savedState.getLongArray(KEY_REPEAT_CHECKED_IDS);
            selectedAlarms = savedState.getLongArray(KEY_SELECTED_ALARMS);
            previousDayMap = savedState.getBundle(KEY_PREVIOUS_DAY_MAP);
            mSelectedTimerSwitch = savedState.getParcelable(KEY_SELECTED_TIMER_SWITCH);
        }

        mSwitchList = (ListView) v.findViewById(R.id.switches_list);
        mExpandInterpolator = new DecelerateInterpolator(EXPAND_DECELERATION);
        mCollapseInterpolator = new DecelerateInterpolator(COLLAPSE_DECELERATION);

        mSwitchAdapter = new SwitchItemAdapter(getActivity(),
                expandedIds, repeatCheckedIds, selectedAlarms, previousDayMap, mSwitchList);
        mSwitchList.setAdapter(mSwitchAdapter);
        mSwitchList.setVerticalScrollBarEnabled(true);
        mSwitchList.setOnCreateContextMenuListener(this);

        return v;
    }

    @Override
    public void onResume() {
        super.onResume();
        TimePickerDialog tpd = (TimePickerDialog) getChildFragmentManager().
                findFragmentByTag(TimerSwitchUtils.FRAG_TAG_TIME_PICKER);
        if (tpd != null) {
            tpd.setOnTimeSetListener(this);
        }
    }

    public class SwitchItemAdapter extends CursorAdapter {
        private static final int EXPAND_DURATION = 300;
        private static final int COLLAPSE_DURATION = 250;

        private final Context mContext;
        private final LayoutInflater mFactory;
        private final String[] mShortWeekDayStrings;
        private final String[] mLongWeekDayStrings;
        private final int mColorLit;
        private final int mColorDim;
        private final int mBackgroundColorExpanded;
        private final int mBackgroundColor;
        private final Typeface mRobotoNormal;
        private final Typeface mRobotoBold;
        private final ListView mList;

        private final HashSet<Long> mExpanded = new HashSet<Long>();
        private final HashSet<Long> mRepeatChecked = new HashSet<Long>();
        private final HashSet<Long> mSelectedSwitches = new HashSet<Long>();
        private Bundle mPreviousDaysOfWeekMap = new Bundle();

        private final int mCollapseExpandHeight;

        // This determines the order in which it is shown and processed in the
        // UI.
        private final int[] DAY_ORDER = new int[] {
                Calendar.SUNDAY,
                Calendar.MONDAY,
                Calendar.TUESDAY,
                Calendar.WEDNESDAY,
                Calendar.THURSDAY,
                Calendar.FRIDAY,
                Calendar.SATURDAY,
        };

        public class ItemHolder {

            // views for optimization
            LinearLayout timerSwitchItem;
            TextTime clock;
            Switch onoff;
            TextView daysOfWeek;
            TextView label;
            ImageView reset;
            View expandArea;
            View summary;
            CheckBox repeat;
            LinearLayout repeatDays;
            ViewGroup[] dayButtonParents = new ViewGroup[DAYS_COUNT_WEEK];
            Button[] dayButtons = new Button[DAYS_COUNT_WEEK];
            View hairLine;
            View arrow;
            View collapseExpandArea;

            // Other states
            TimerSwitch switcher;
        }

        // Used for scrolling an expanded item in the list to make sure it is
        // fully visible.
        private long mScrollAlarmId = -1;
        private final Runnable mScrollRunnable = new Runnable() {
            @Override
            public void run() {
                if (mScrollAlarmId != -1) {
                    View v = getViewById(mScrollAlarmId);
                    if (v != null) {
                        Rect rect = new Rect(v.getLeft(), v.getTop(), v.getRight(), v.getBottom());
                        mList.requestChildRectangleOnScreen(v, rect, false);
                    }
                    mScrollAlarmId = -1;
                }
            }
        };

        public SwitchItemAdapter(Context context, long[] expandedIds, long[] repeatCheckedIds,
                long[] selectedAlarms, Bundle previousDaysOfWeekMap, ListView list) {
            super(context, null, 0);
            mContext = context;
            mFactory = LayoutInflater.from(context);
            mList = list;

            DateFormatSymbols dfs = new DateFormatSymbols();
            mShortWeekDayStrings = Utils.getShortWeekdays();
            mLongWeekDayStrings = dfs.getWeekdays();

            Resources res = mContext.getResources();
            mColorLit = res.getColor(R.color.clock_white);
            mColorDim = res.getColor(R.color.dark_green_title);
            mBackgroundColorExpanded = res.getColor(R.color.switch_whiteish);
            mBackgroundColor = R.drawable.alarm_background_normal;

            mRobotoBold = Typeface.create("sans-serif-condensed", Typeface.BOLD);
            mRobotoNormal = Typeface.create("sans-serif-condensed", Typeface.NORMAL);

            if (expandedIds != null) {
                buildHashSetFromArray(expandedIds, mExpanded);
            }
            if (repeatCheckedIds != null) {
                buildHashSetFromArray(repeatCheckedIds, mRepeatChecked);
            }
            if (previousDaysOfWeekMap != null) {
                mPreviousDaysOfWeekMap = previousDaysOfWeekMap;
            }
            if (selectedAlarms != null) {
                buildHashSetFromArray(selectedAlarms, mSelectedSwitches);
            }

            mCollapseExpandHeight = (int) res.getDimension(R.dimen.collapse_expand_height);
        }

        private View getViewById(long id) {
            for (int i = 0; i < mList.getCount(); i++) {
                View v = mList.getChildAt(i);
                if (v != null) {
                    ItemHolder h = (ItemHolder) (v.getTag());
                    if (h != null && h.switcher.id == id) {
                        return v;
                    }
                }
            }
            return null;
        }

        private void setNewHolder(View view) {
            // standard view holder optimization
            final ItemHolder holder = new ItemHolder();
            holder.timerSwitchItem = (LinearLayout) view.findViewById(R.id.switch_item);
            holder.clock = (TextTime) view.findViewById(R.id.digital_clock);
            holder.onoff = (Switch) view.findViewById(R.id.onoff);
            holder.onoff.setTypeface(mRobotoNormal);
            holder.daysOfWeek = (TextView) view.findViewById(R.id.daysOfWeek);
            holder.label = (TextView) view.findViewById(R.id.label);
            holder.reset = (ImageView) view.findViewById(R.id.reset);
            holder.summary = view.findViewById(R.id.summary);
            holder.expandArea = view.findViewById(R.id.expand_area);
            holder.hairLine = view.findViewById(R.id.hairline);
            holder.arrow = view.findViewById(R.id.arrow);
            holder.repeat = (CheckBox) view.findViewById(R.id.repeat_onoff);
            holder.repeatDays = (LinearLayout) view.findViewById(R.id.repeat_days);
            holder.collapseExpandArea = view.findViewById(R.id.collapse_expand);

            // Build button for each day.
            for (int i = 0; i < DAYS_COUNT_WEEK; i++) {
                final Button dayButton = (Button) mFactory.inflate(
                        R.layout.day_button, holder.repeatDays, false /* attachToRoot */);
                dayButton.setText(mShortWeekDayStrings[i]);
                dayButton.setContentDescription(mLongWeekDayStrings[DAY_ORDER[i]]);
                holder.repeatDays.addView(dayButton);
                holder.dayButtons[i] = dayButton;
            }

            view.setTag(holder);
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            final View view = mFactory.inflate(R.layout.timerswitch_item, parent, false);
            setNewHolder(view);
            return view;
        }

        @Override
        public void bindView(final View view, Context context, Cursor cursor) {
            final TimerSwitch timerSwitch = new TimerSwitch(cursor);
            Object tag = view.getTag();
            if (tag == null) {
                // The view was converted but somehow lost its tag.
                setNewHolder(view);
            }
            final ItemHolder itemHolder = (ItemHolder) tag;
            itemHolder.switcher = timerSwitch;

            itemHolder.clock.setFormat(
                    (int) mContext.getResources().getDimension(R.dimen.switch_label_size));
            itemHolder.clock.setTime(timerSwitch.hour, timerSwitch.minutes);
            itemHolder.clock.setClickable(true);
            itemHolder.clock.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    mSelectedTimerSwitch = itemHolder.switcher;
                    TimerSwitchUtils.showTimeEditDialog(getChildFragmentManager(),
                            timerSwitch, TimerSwitchSettingsFragment.this
                            , DateFormat.is24HourFormat(getActivity()));
                    expandTimerSwitch(itemHolder, true);
                    itemHolder.timerSwitchItem.post(mScrollRunnable);
                }
            });

            // We must unset the listener first because this maybe a recycled
            // view so changing the
            // state would affect the wrong switch.
            itemHolder.onoff.setOnCheckedChangeListener(null);
            itemHolder.onoff.setChecked(timerSwitch.enabled);

            if (mSelectedSwitches.contains(itemHolder.switcher.id)) {
                itemHolder.timerSwitchItem.setBackgroundColor(mBackgroundColorExpanded);
                setItemAlpha(itemHolder, true);
                itemHolder.onoff.setEnabled(false);
            } else {
                itemHolder.onoff.setEnabled(true);
                itemHolder.timerSwitchItem.setBackgroundResource(mBackgroundColor);
                setItemAlpha(itemHolder, itemHolder.onoff.isChecked());
            }

            final CompoundButton.OnCheckedChangeListener onOffListener =
                    new CompoundButton.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton compoundButton,
                                boolean checked) {
                            Log.d("OnChecked:" + checked);
                            if (checked != timerSwitch.enabled) {
                                setItemAlpha(itemHolder, checked);
                                timerSwitch.enabled = checked;
                                asyncUpdateTimerSwitch(timerSwitch, timerSwitch.enabled);
                            }
                        }
                    };

            itemHolder.onoff.setOnCheckedChangeListener(onOffListener);

            String labelSpace = "";
            // Set the repeat text or leave it blank if it does not repeat.
            final String daysOfWeekStr =
                    timerSwitch.daysOfWeek.toString(TimerSwitchSettingsFragment.this.getActivity(),
                            false);
            if (daysOfWeekStr != null && daysOfWeekStr.length() != 0) {
                itemHolder.daysOfWeek.setText(daysOfWeekStr);
                itemHolder.daysOfWeek.setContentDescription(timerSwitch.daysOfWeek
                        .toAccessibilityString(
                        TimerSwitchSettingsFragment.this.getActivity()));
                itemHolder.daysOfWeek.setVisibility(View.VISIBLE);
                labelSpace = "  ";
                itemHolder.daysOfWeek.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        expandTimerSwitch(itemHolder, true);
                        itemHolder.timerSwitchItem.post(mScrollRunnable);
                    }
                });

            } else {
                itemHolder.daysOfWeek.setVisibility(View.GONE);
            }

            if (timerSwitch.id == TimerSwitchConstants.SWITCH_ON) {
                itemHolder.label.setText(mContext.getResources().getString(R.string.switch_on));
                if (getResources().getBoolean(
                        R.bool.config_schedule_power_on_off_tlcl)) {
                    itemHolder.clock.setCompoundDrawablesWithIntrinsicBounds(
                            R.drawable.ic_power_on, 0, 0, 0);
                }
            } else if (timerSwitch.id == TimerSwitchConstants.SWITCH_OFF) {
                itemHolder.label.setText(mContext.getResources().getString(R.string.switch_off));
                if (getResources().getBoolean(
                        R.bool.config_schedule_power_on_off_tlcl)) {
                    itemHolder.clock.setCompoundDrawablesWithIntrinsicBounds(
                            R.drawable.ic_power_off, 0, 0, 0);
                }
            }

            itemHolder.reset.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    asyncResetTimerSwtich(timerSwitch, view);
                    Log.d("Reset TimerSwitch");
                }
            });

            boolean expanded = isTimerSwitchExpanded(timerSwitch);
            itemHolder.expandArea.setVisibility(expanded ? View.VISIBLE : View.GONE);
            itemHolder.summary.setVisibility(expanded ? View.GONE : View.VISIBLE);

            if (expanded) {
                expandTimerSwitch(itemHolder, false);
            } else {
                collapseTimerSwitch(itemHolder, false);
            }

            itemHolder.timerSwitchItem.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (isTimerSwitchExpanded(timerSwitch)) {
                        collapseTimerSwitch(itemHolder, true);
                    } else {
                        expandTimerSwitch(itemHolder, true);
                    }
                }
            });
        }

        private void bindExpandArea(final ItemHolder itemHolder, final TimerSwitch timerSwitch) {
            // Views in here are not bound until the item is expanded.
            if (mRepeatChecked.contains(timerSwitch.id)
                    || itemHolder.switcher.daysOfWeek.isRepeating()) {
                itemHolder.repeat.setChecked(true);
                itemHolder.repeatDays.setVisibility(View.VISIBLE);
            } else {
                itemHolder.repeat.setChecked(false);
                itemHolder.repeatDays.setVisibility(View.GONE);
            }
            itemHolder.repeat.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    final boolean checked = ((CheckBox) view).isChecked();
                    if (checked) {
                        // Show days
                        itemHolder.repeatDays.setVisibility(View.VISIBLE);
                        mRepeatChecked.add(timerSwitch.id);

                        // Set all previously set days
                        // or
                        // Set all days if no previous.
                        final int bitSet = mPreviousDaysOfWeekMap.getInt("" + timerSwitch.id);
                        timerSwitch.daysOfWeek.setBitSet(bitSet);
                        if (!timerSwitch.daysOfWeek.isRepeating()) {
                            timerSwitch.daysOfWeek.setDaysOfWeek(true, DAY_ORDER);
                        }
                        updateDaysOfWeekButtons(itemHolder, timerSwitch.daysOfWeek);
                    } else {
                        itemHolder.repeatDays.setVisibility(View.GONE);
                        mRepeatChecked.remove(timerSwitch.id);

                        // Remember the set days in case the user wants it back.
                        final int bitSet = timerSwitch.daysOfWeek.getBitSet();
                        mPreviousDaysOfWeekMap.putInt("" + timerSwitch.id, bitSet);

                        // Remove all repeat days
                        timerSwitch.daysOfWeek.clearAllDays();
                    }
                    asyncUpdateTimerSwitch(timerSwitch, false);
                }
            });

            updateDaysOfWeekButtons(itemHolder, timerSwitch.daysOfWeek);
            for (int i = 0; i < DAYS_COUNT_WEEK; i++) {
                final int buttonIndex = i;

                itemHolder.dayButtons[i].setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        final boolean isActivated =
                                itemHolder.dayButtons[buttonIndex].isActivated();
                        timerSwitch.daysOfWeek.setDaysOfWeek(!isActivated, DAY_ORDER[buttonIndex]);
                        if (!isActivated) {
                            turnOnDayOfWeek(itemHolder, buttonIndex);
                        } else {
                            turnOffDayOfWeek(itemHolder, buttonIndex);

                            // See if this was the last day, if so, un-check the
                            // repeat box.
                            if (!timerSwitch.daysOfWeek.isRepeating()) {
                                itemHolder.repeatDays.setVisibility(View.GONE);
                                itemHolder.repeat.setTextColor(mColorDim);
                                mRepeatChecked.remove(timerSwitch.id);

                                // Set history to no days, so it will be
                                // everyday when repeat is
                                // turned back on
                                mPreviousDaysOfWeekMap.putInt("" + timerSwitch.id,
                                        DaysOfWeek.NO_DAYS_SET);
                            }
                        }
                        asyncUpdateTimerSwitch(timerSwitch, false);
                    }
                });
            }
        }

        /**
         * Expands the switch for editing.
         *
         * @param itemHolder The item holder instance.
         */
        private void expandTimerSwitch(final ItemHolder itemHolder, boolean animate) {
            mExpanded.add(itemHolder.switcher.id);
            bindExpandArea(itemHolder, itemHolder.switcher);
            // Scroll the view to make sure it is fully viewed
            mScrollAlarmId = itemHolder.switcher.id;

            // Save the starting height so we can animate from this value.
            final int startingHeight = itemHolder.timerSwitchItem.getHeight();

            // Set the expand area to visible so we can measure the height to
            // animate to.
            itemHolder.timerSwitchItem.setBackgroundColor(mBackgroundColorExpanded);
            itemHolder.expandArea.setVisibility(View.VISIBLE);

            if (!animate) {
                // Set the "end" layout and don't do the animation.
                itemHolder.arrow.setRotation(180);
                // We need to translate the hairline up, so the height of the
                // collapseArea
                // needs to be measured to know how high to translate it.
                final ViewTreeObserver observer = mSwitchList.getViewTreeObserver();
                observer.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                    @Override
                    public boolean onPreDraw() {
                        // We don't want to continue getting called for every
                        // listview drawing.
                        if (observer.isAlive()) {
                            observer.removeOnPreDrawListener(this);
                        }
                        int hairlineHeight = itemHolder.hairLine.getHeight();
                        int collapseHeight =
                                itemHolder.collapseExpandArea.getHeight() - hairlineHeight;
                        itemHolder.hairLine.setTranslationY(-collapseHeight);
                        return true;
                    }
                });
                return;
            }

            // Add an onPreDrawListener, which gets called after measurement but
            // before the draw.
            // This way we can check the height we need to animate to before any
            // drawing.
            // Note the series of events:
            // * expandArea is set to VISIBLE, which causes a layout pass
            // * the view is measured, and our onPreDrawListener is called
            // * we set up the animation using the start and end values.
            // * the height is set back to the starting point so it can be
            // animated down.
            // * request another layout pass.
            // * return false so that onDraw() is not called for the single
            // frame before
            // the animations have started.
            final ViewTreeObserver observer = mSwitchList.getViewTreeObserver();
            observer.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                    // We don't want to continue getting called for every
                    // listview drawing.
                    if (observer.isAlive()) {
                        observer.removeOnPreDrawListener(this);
                    }
                    // Calculate some values to help with the animation.
                    final int endingHeight = itemHolder.timerSwitchItem.getHeight();
                    final int distance = endingHeight - startingHeight;
                    final int collapseHeight = itemHolder.collapseExpandArea.getHeight();
                    int hairlineHeight = itemHolder.hairLine.getHeight();
                    final int hairlineDistance = collapseHeight - hairlineHeight;

                    // Set the height back to the start state of the animation.
                    itemHolder.timerSwitchItem.getLayoutParams().height = startingHeight;
                    // To allow the expandArea to glide in with the expansion
                    // animation, set a
                    // negative top margin, which will animate down to a margin
                    // of 0 as the height
                    // is increased.
                    // Note that we need to maintain the bottom margin as a
                    // fixed value (instead of
                    // just using a listview, to allow for a flatter hierarchy)
                    // to fit the bottom
                    // bar underneath.
                    FrameLayout.LayoutParams expandParams = (FrameLayout.LayoutParams)
                            itemHolder.expandArea.getLayoutParams();
                    expandParams.setMargins(0, -distance, 0, collapseHeight);
                    itemHolder.timerSwitchItem.requestLayout();

                    // Set up the animator to animate the expansion.
                    ValueAnimator animator = ValueAnimator.ofFloat(0f, 1f)
                            .setDuration(EXPAND_DURATION);
                    animator.setInterpolator(mExpandInterpolator);
                    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
                        @Override
                        public void onAnimationUpdate(ValueAnimator animator) {
                            Float value = (Float) animator.getAnimatedValue();

                            // For each value from 0 to 1, animate the various
                            // parts of the layout.
                            itemHolder.timerSwitchItem.getLayoutParams().height =
                                    (int) (value * distance + startingHeight);
                            FrameLayout.LayoutParams expandParams = (FrameLayout.LayoutParams)
                                    itemHolder.expandArea.getLayoutParams();
                            expandParams.setMargins(
                                    0, (int) -((1 - value) * distance), 0, collapseHeight);
                            itemHolder.arrow.setRotation(180 * value);
                            itemHolder.hairLine.setTranslationY(-hairlineDistance * value);
                            itemHolder.summary.setAlpha(1 - value);

                            itemHolder.timerSwitchItem.requestLayout();
                        }
                    });
                    // Set everything to their final values when the animation's
                    // done.
                    animator.addListener(new AnimatorListener() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            // Set it back to wrap content since we'd explicitly
                            // set the height.
                            itemHolder.timerSwitchItem.getLayoutParams().height =
                                    LayoutParams.WRAP_CONTENT;
                            itemHolder.arrow.setRotation(180);
                            itemHolder.hairLine.setTranslationY(-hairlineDistance);
                            itemHolder.summary.setVisibility(View.GONE);
                        }

                        @Override
                        public void onAnimationCancel(Animator animation) {
                            // TODO we may have to deal with cancelations of the
                            // animation.
                        }

                        @Override
                        public void onAnimationRepeat(Animator animation) {
                        }

                        @Override
                        public void onAnimationStart(Animator animation) {
                        }
                    });
                    animator.start();

                    // Return false so this draw does not occur to prevent the
                    // final frame from
                    // being drawn for the single frame before the animations
                    // start.
                    return false;
                }
            });
        }

        private void collapseTimerSwitch(final ItemHolder itemHolder, boolean animate) {
            mExpanded.remove(itemHolder.switcher.id);

            // Save the starting height so we can animate from this value.
            final int startingHeight = itemHolder.timerSwitchItem.getHeight();

            // Set the expand area to gone so we can measure the height to
            // animate to.
            itemHolder.timerSwitchItem.setBackgroundResource(mBackgroundColor);
            itemHolder.expandArea.setVisibility(View.GONE);

            if (!animate) {
                // Set the "end" layout and don't do the animation.
                itemHolder.arrow.setRotation(0);
                itemHolder.hairLine.setTranslationY(0);
                return;
            }

            // Add an onPreDrawListener, which gets called after measurement but
            // before the draw.
            // This way we can check the height we need to animate to before any
            // drawing.
            // Note the series of events:
            // * expandArea is set to GONE, which causes a layout pass
            // * the view is measured, and our onPreDrawListener is called
            // * we set up the animation using the start and end values.
            // * expandArea is set to VISIBLE again so it can be shown
            // animating.
            // * request another layout pass.
            // * return false so that onDraw() is not called for the single
            // frame before
            // the animations have started.
            final ViewTreeObserver observer = mSwitchList.getViewTreeObserver();
            observer.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                    if (observer.isAlive()) {
                        observer.removeOnPreDrawListener(this);
                    }

                    // Calculate some values to help with the animation.
                    final int endingHeight = itemHolder.timerSwitchItem.getHeight();
                    final int distance = endingHeight - startingHeight;
                    int hairlineHeight = itemHolder.hairLine.getHeight();
                    final int hairlineDistance = mCollapseExpandHeight - hairlineHeight;

                    // Re-set the visibilities for the start state of the
                    // animation.
                    itemHolder.expandArea.setVisibility(View.VISIBLE);
                    itemHolder.summary.setVisibility(View.VISIBLE);
                    itemHolder.summary.setAlpha(1);

                    // Set up the animator to animate the expansion.
                    ValueAnimator animator = ValueAnimator.ofFloat(0f, 1f)
                            .setDuration(COLLAPSE_DURATION);
                    animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
                        @Override
                        public void onAnimationUpdate(ValueAnimator animator) {
                            Float value = (Float) animator.getAnimatedValue();

                            // For each value from 0 to 1, animate the various
                            // parts of the layout.
                            itemHolder.timerSwitchItem.getLayoutParams().height =
                                    (int) (value * distance + startingHeight);
                            FrameLayout.LayoutParams expandParams = (FrameLayout.LayoutParams)
                                    itemHolder.expandArea.getLayoutParams();
                            expandParams.setMargins(
                                    0, (int) (value * distance), 0, mCollapseExpandHeight);
                            itemHolder.arrow.setRotation(180 * (1 - value));
                            itemHolder.hairLine.setTranslationY(-hairlineDistance * (1 - value));
                            itemHolder.summary.setAlpha(value);

                            itemHolder.timerSwitchItem.requestLayout();
                        }
                    });
                    animator.setInterpolator(mCollapseInterpolator);
                    // Set everything to their final values when the animation's
                    // done.
                    animator.addListener(new AnimatorListener() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            // Set it back to wrap content since we'd explicitly
                            // set the height.
                            itemHolder.timerSwitchItem.getLayoutParams().height =
                                    LayoutParams.WRAP_CONTENT;

                            FrameLayout.LayoutParams expandParams = (FrameLayout.LayoutParams)
                                    itemHolder.expandArea.getLayoutParams();
                            expandParams.setMargins(0, 0, 0, mCollapseExpandHeight);

                            itemHolder.expandArea.setVisibility(View.GONE);
                            itemHolder.arrow.setRotation(0);
                            itemHolder.hairLine.setTranslationY(0);
                        }

                        @Override
                        public void onAnimationCancel(Animator animation) {
                            // TODO we may have to deal with cancelations of the
                            // animation.
                        }

                        @Override
                        public void onAnimationRepeat(Animator animation) {
                        }

                        @Override
                        public void onAnimationStart(Animator animation) {
                        }
                    });
                    animator.start();

                    return false;
                }
            });
        }

        // Sets the alpha of the item except the on/off switch. This gives a
        // visual effect
        // for enabled/disabled switch while leaving the on/off switch more
        // visible
        private void setItemAlpha(ItemHolder holder, boolean enabled) {
            float alpha = enabled ? 1f : 0.5f;
            holder.clock.setAlpha(alpha);
        }

        private void updateDaysOfWeekButtons(ItemHolder holder, DaysOfWeek daysOfWeek) {
            HashSet<Integer> setDays = daysOfWeek.getSetDays();
            for (int i = 0; i < DAYS_COUNT_WEEK; i++) {
                if (setDays.contains(DAY_ORDER[i])) {
                    turnOnDayOfWeek(holder, i);
                } else {
                    turnOffDayOfWeek(holder, i);
                }
            }
        }

        public long[] getExpandedArray() {
            int index = 0;
            long[] ids = new long[mExpanded.size()];
            for (long id : mExpanded) {
                ids[index] = id;
                index++;
            }
            return ids;
        }

        private void turnOffDayOfWeek(ItemHolder holder, int dayIndex) {
            final Button dayButton = holder.dayButtons[dayIndex];
            dayButton.setActivated(false);
            dayButton.setTextColor(mColorDim);
        }

        private void turnOnDayOfWeek(ItemHolder holder, int dayIndex) {
            final Button dayButton = holder.dayButtons[dayIndex];
            dayButton.setActivated(true);
            dayButton.setTextColor(mColorLit);
        }

        public long[] getSelectedAlarmsArray() {
            int index = 0;
            long[] ids = new long[mSelectedSwitches.size()];
            for (long id : mSelectedSwitches) {
                ids[index] = id;
                index++;
            }
            return ids;
        }

        public long[] getRepeatArray() {
            int index = 0;
            long[] ids = new long[mRepeatChecked.size()];
            for (long id : mRepeatChecked) {
                ids[index] = id;
                index++;
            }
            return ids;
        }

        public Bundle getPreviousDaysOfWeekMap() {
            return mPreviousDaysOfWeekMap;
        }

        private void buildHashSetFromArray(long[] ids, HashSet<Long> set) {
            for (long id : ids) {
                set.add(id);
            }
        }

        private boolean isTimerSwitchExpanded(TimerSwitch timerSwitch) {
            return mExpanded.contains(timerSwitch.id);
        }

    }

    private void asyncUpdateTimerSwitch(final TimerSwitch timerSwitch, final boolean popToast) {
        final Context context = TimerSwitchSettingsFragment.this.getActivity()
                .getApplicationContext();
        final AsyncTask<Void, Void, TimerSwitch> updateTask =
                new AsyncTask<Void, Void, TimerSwitch>() {
                    @Override
                    protected TimerSwitch doInBackground(Void... parameters) {
                        // dismiss all expired instances of timerswitch
                        TimerSwitchUtils.deleteAllExpiredTimerSwitch(context, timerSwitch.id);

                        //update timerswitch db
                        TimerSwitchUtils.updateTimerSwitch(context.getContentResolver(),
                                timerSwitch);

                        if (timerSwitch.enabled) {
                            // enable timerswitch
                            TimerSwitchUtils.enableTimerSwitch(context, timerSwitch);
                            return timerSwitch;
                        }
                        return null;
                    }

                    @Override
                    protected void onPostExecute(TimerSwitch instance) {
                        if (popToast && instance != null) {
                            Log.v("asyncUpdateTimerSwitch:" + popToast + " instance:" + instance);
                            long time = instance.switchtime;
                            if (timerSwitch.daysOfWeek.isRepeating()) {
                                time = TimerSwitchUtils.calculateTimerSwitch(instance.hour,
                                        instance.minutes, instance.daysOfWeek).getTimeInMillis();
                            }
                            TimerSwitchUtils.popAlarmSetToast(context, time, instance.id);
                        }
                    }
                };
        updateTask.execute();
    }

    private void asyncResetTimerSwtich(final TimerSwitch timerSwitch, final View viewToReset) {
        final Context context = TimerSwitchSettingsFragment.this.getActivity()
                .getApplicationContext();
        final AsyncTask<Void, Void, Void> resetTask = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... parameters) {
                // Activity may be closed at this point , make sure data is
                // still valid
                if (context != null && timerSwitch != null) {
                    Log.d("reset TimerSwitch");
                    // disable current timerswitch
                    TimerSwitchUtils.disableTimerSwitch(context, timerSwitch);
                    TimerSwitchUtils.resetTimerSwitch(context, timerSwitch);
                }
                return null;
            }
        };
        resetTask.execute();
    }

    @Override
    public void onTimeSet(RadialPickerLayout view, int hourOfDay, int minute) {
        if (mSelectedTimerSwitch != null) {
            mSelectedTimerSwitch.hour = hourOfDay;
            mSelectedTimerSwitch.minutes = minute;
            mSelectedTimerSwitch.enabled = true;
            asyncUpdateTimerSwitch(mSelectedTimerSwitch, true);
            mSelectedTimerSwitch = null;
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        return TimerSwitchUtils.getAlarmsCursorLoader(getActivity());
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor cursor) {
        // TODO Auto-generated method stub
        mSwitchAdapter.swapCursor(cursor);
    }

    @Override
    public void onLoaderReset(Loader<Cursor> loader) {
        // TODO Auto-generated method stub
        mSwitchAdapter.swapCursor(null);
    }

}
