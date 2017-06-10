/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.bttest;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ListFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.AdapterView.OnItemSelectedListener;

import com.qti.bttest.Utils.RangeContent;
import com.qti.bttest.Utils.SelectionsContent;
import com.qti.bttest.Utils.TestItem;
import com.qti.bttest.Utils.ValueContent;

import java.util.ArrayList;
import java.util.HashMap;

public class TestContentFragment extends ListFragment {
    private static final String TAG = "TestContentFragment";

    private static final String DIALOG_TAG_EDIT = "edit_dialog";

    private View mRootView;
    private LayoutInflater mInflater;

    private ListView mList;
    private MyArrayAdapter mAdapter;

    private int mTestId;
    private SelectionsContent mSelections;
    private OnSelectionChangedListener mListener = null;
    private TestItem[] mTestItems;
    private HashMap<TestItem, ArrayList<ValueContent>> mTestItemsList;

    private OnCheckedChangeListener mCheckedChangedListener = new OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            int itemId = (Integer) buttonView.getTag();
            TestItem item = mTestItems[itemId];
            Log.d(TAG, "Value changed, item: " + item + ", new value: " + isChecked);

            ArrayList<ValueContent> list = mTestItemsList.get(item);
            for (int i = 0; i < list.size(); i++) {
                ValueContent value = list.get(i);
                if (isChecked == Boolean.valueOf(value._value)) {
                    updateSelectionsContent(false, itemId, item, value);
                    return;
                }
            }
        }
    };

    private OnItemSelectedListener mValueChangedListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            int itemId = (Integer) parent.getTag();
            ValueContent value = ValueContent.getSpinnerContentValue((Spinner) parent, position);
            Log.d(TAG, "Value changed, item: " + mTestItems[itemId]
                    + ", new value: " + value._value);

            updateSelectionsContent(false, itemId, mTestItems[itemId], value);
        }
        @Override
        public void onNothingSelected(AdapterView<?> parent) {
            // Do nothing.
        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = super.onCreateView(inflater, container, savedInstanceState);
        mInflater = inflater;

        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle bundle) {
        mList = getListView();
        mList.setDividerHeight(0);

        buildTestItems();

        super.onActivityCreated(bundle);
    }

    public void onTestModeChanged(int newTestId) {
        mTestId = newTestId;
        if (mSelections == null) {
            mSelections = new SelectionsContent(mTestId);
        } else {
            mSelections.reset(mTestId);
        }

        buildTestItems();
    }

    public void setOnSelectionChangedListener(OnSelectionChangedListener listener) {
        mListener = listener;
    }

    private void buildTestItems() {
        if (mTestItemsList == null) {
            mTestItemsList = new HashMap<TestItem, ArrayList<ValueContent>>();
        }

        mTestItemsList.clear();
        mTestItems = Utils.buildTestItems(getActivity(), mTestId, mTestItemsList);

        // Set the adapter to the list.
        mAdapter = new MyArrayAdapter(this, getActivity(), R.layout.test_item_spinner, mTestItems);
        setListAdapter(mAdapter);
    }


    private void updateSelectionsContent(boolean init, int itemId, TestItem item,
            ValueContent curValue) {
        if (curValue == null) return;

        // Update the selection.
        if (mSelections == null) {
            mSelections = new SelectionsContent(mTestId);
        }
        mSelections.setSelection(item, curValue);

        // Notify the selection change.
        if (mListener != null) {
            mListener.onSelectionChanged(mSelections, item);
        }

        // Update the test items. If has the update, need invalidate the list.
        if (!init) {
            if (Utils.updateTestItems(getActivity(), mTestId, mTestItemsList, item, curValue)) {
                // Caused by the item has been changed, so need update the selections.
                mSelections.setSelection(item._relationItem, null);
            }
            mAdapter.notifyDataSetChanged();
        }
    }

    private static class MyArrayAdapter extends ArrayAdapter<TestItem> {
        private TestContentFragment fragment;

        public MyArrayAdapter(TestContentFragment fragment, Context context, int resource,
                TestItem[] items) {
            super(context, resource, items);
            this.fragment = fragment;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View v = null;

            TestItem item = fragment.mTestItems[position];
            switch (item._layout_type) {
                case Utils.LAYOUT_TYPE_SWITCH:
                    v = fragment.mInflater.inflate(R.layout.test_item_switch, null);
                    setSwitchValue(position, v);
                    return v;
                case Utils.LAYOUT_TYPE_EDITTEXT:
                    v = fragment.mInflater.inflate(R.layout.test_item_edittext, null);
                    setEditTextView(position, v);
                    return v;
                case Utils.LAYOUT_TYPE_SPINNER:
                    v = fragment.mInflater.inflate(R.layout.test_item_spinner, null);
                    setSpinnerValues(position, v);
                    return v;
            }

            return new View(fragment.getActivity());
        }

        private void setSwitchValue(int position, View v) {
            if (v == null) return;

            TestItem item = fragment.mTestItems[position];

            TextView testItemName = (TextView) v.findViewById(R.id.test_item_name);
            Switch switchView = (Switch) v.findViewById(R.id.test_item_value);

            // Set test item name.
            testItemName.setText(item._label);

            // Set the tag and default value fot the item.
            switchView.setTag(position);

            boolean curValue;
            ValueContent curValueContent = fragment.mSelections == null ? null
                    : fragment.mSelections.getCurValue(item);
            if (curValueContent == null) {
                ArrayList<ValueContent> values = fragment.mTestItemsList.get(item);
                curValue = Boolean.valueOf(values.get(0)._value);
                fragment.updateSelectionsContent(true, position, item, values.get(0));
            } else {
                curValue = Boolean.valueOf(curValueContent._value);
            }
            switchView.setChecked(curValue);
            switchView.setOnCheckedChangeListener(fragment.mCheckedChangedListener);
        }

        private void setEditTextView(final int position, View v) {
            if (v == null) return;

            LinearLayout itemContentView = (LinearLayout) v.findViewById(R.id.test_item_content);

            final TestItem item = fragment.mTestItems[position];
            final ArrayList<ValueContent> values = fragment.mTestItemsList.get(item);
            ValueContent curValueContent = fragment.mSelections == null ? null
                    : fragment.mSelections.getCurValue(item);
            if (curValueContent == null) {
                curValueContent = Utils.buildValueFromRanges(true, values);
                fragment.updateSelectionsContent(true, position, item, curValueContent);
            }

            final String[] curValues = Utils.getValueArray(curValueContent._value);
            if (curValues.length != values.size()) {
                Log.e(TAG, "set edit text view, but the selections value didn't match.");
                return;
            }

            // New the testItemName view and add to the item content view.
            TextView testItemName = new TextView(getContext());
            testItemName.setTextAppearance(getContext(), android.R.style.TextAppearance_Medium);
            testItemName.setText(item._label);
            if (values.size() == 1) {
                itemContentView.addView(testItemName, new LinearLayout.LayoutParams(
                        0, LayoutParams.WRAP_CONTENT, 1));
            } else {
                // If the edit is more than one
                itemContentView.addView(testItemName, new LinearLayout.LayoutParams(
                        LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
                testItemName.setPadding(0, 0, 10, 0);
            }

            for (int i = 0; i < values.size(); i++) {
                TextView edit = new TextView(getContext());
                edit.setGravity(Gravity.CENTER);
                edit.setBackgroundResource(R.drawable.edit_bg);
                edit.setFocusable(true);
                edit.setClickable(true);
                edit.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        MyDialog dialog = MyDialog.newInstance(fragment, item, values,
                                curValues, position);
                        dialog.show(fragment.getFragmentManager(), DIALOG_TAG_EDIT);
                    }
                });

                // Set the default value.
                edit.setText(curValues[i]);
                itemContentView.addView(edit, new LinearLayout.LayoutParams(
                        0, LayoutParams.WRAP_CONTENT, 1));
            }
        }

        private void setSpinnerValues(int position, View v) {
            if (v == null) return;

            TestItem item = fragment.mTestItems[position];

            TextView testItemName = (TextView) v.findViewById(R.id.test_item_name);
            Spinner spinnerView = (Spinner) v.findViewById(R.id.test_item_value);

            // Set test item name.
            testItemName.setText(item._label);

            // Set the tag and default value for the item.
            spinnerView.setTag(position);

            String curValue;
            ValueContent curValueContent = fragment.mSelections == null ? null
                    : fragment.mSelections.getCurValue(item);
            ArrayList<ValueContent> values = fragment.mTestItemsList.get(item);
            if (curValueContent == null) {
                curValue = values.get(0)._value;
                fragment.updateSelectionsContent(true, position, item, values.get(0));
            } else {
                curValue = curValueContent._value;
            }

            ArrayAdapter<ValueContent> adapter = new ArrayAdapter<ValueContent>(
                    fragment.getActivity(), R.layout.spinner_item, values);
            spinnerView.setAdapter(adapter);
            ValueContent.setSpinnerContentValue(spinnerView, curValue);
            spinnerView.setOnItemSelectedListener(fragment.mValueChangedListener);
        }
    }

    public static class MyDialog extends DialogFragment {
        private static final String KEY_TEST_ITEM = "test_item";
        private static final String KEY_VALUES = "values";
        private static final String KEY_CUR_VALUE = "cur_value";
        private static final String KEY_POSITION = "position";

        public static MyDialog newInstance(TestContentFragment fragment, TestItem item,
                ArrayList<ValueContent> values, String[] curValue, int position) {
            MyDialog dialog = new MyDialog();
            dialog.setTargetFragment(fragment, 0);

            Bundle bundle = new Bundle();
            bundle.putParcelable(KEY_TEST_ITEM, item);
            bundle.putParcelableArrayList(KEY_VALUES, values);
            bundle.putStringArray(KEY_CUR_VALUE, curValue);
            bundle.putInt(KEY_POSITION, position);
            dialog.setArguments(bundle);

            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Bundle arg = getArguments();
            final TestContentFragment target = (TestContentFragment) getTargetFragment();
            final TestItem item = arg.getParcelable(KEY_TEST_ITEM);
            final ArrayList<ValueContent> values = arg.getParcelableArrayList(KEY_VALUES);
            final String[] curValue = arg.getStringArray(KEY_CUR_VALUE);
            final int position = arg.getInt(KEY_POSITION);

            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle(item._label)
                   .setView(buildView(item, values, curValue))
                   .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            ValueContent curValueContent = Utils.buildValueFromRanges(false,
                                    values);
                            target.updateSelectionsContent(false, position, item, curValueContent);
                            dialog.dismiss();
                        }
                    })
                   .setNegativeButton(android.R.string.cancel, null);
            return builder.create();
        }

        private View buildView(final TestItem item, final ArrayList<ValueContent> values,
                final String[] curValues) {
            // Build the edit view.
            if (curValues.length != values.size()) {
                Log.e(TAG, "build view for dialog, the values' length is not match.");
                return null;
            }

            LinearLayout itemContentView = (LinearLayout) getActivity().getLayoutInflater()
                    .inflate(R.layout.test_item_edittext, null);
            itemContentView.setPadding(20, 20, 20, 40);

            for (int i = 0; i < values.size(); i++) {
                final ValueContent valueContent = values.get(i);
                if (valueContent instanceof RangeContent) {
                    final RangeContent rangeContent = (RangeContent) valueContent;
                    // Build the LinearLayout view.
                    final LinearLayout rangeEditView = new LinearLayout(getActivity());
                    rangeEditView.setOrientation(LinearLayout.VERTICAL);
                    rangeEditView.setGravity(Gravity.CENTER);

                    // Build the edit view and set the edit text.
                    final EditText edit = new EditText(getActivity());
                    edit.setGravity(Gravity.CENTER);
                    edit.setTextAppearance(getActivity(),
                            android.R.style.TextAppearance_DeviceDefault_Medium);
                    edit.setText(curValues[i]);
                    edit.setInputType(InputType.TYPE_CLASS_NUMBER);
                    edit.addTextChangedListener(new TextWatcher() {
                        @Override
                        public void onTextChanged(CharSequence s, int start,
                                int before, int count) {
                            // do nothing
                        }

                        @Override
                        public void beforeTextChanged(CharSequence s, int start, int count,
                                int after) {
                            // do nothing
                        }

                        @Override
                        public void afterTextChanged(Editable s) {
                            // Update the error message.
                            boolean enable = true;
                            if (TextUtils.isEmpty(s.toString())) {
                                edit.setError(getActivity().getText(
                                        R.string.error_msg_is_null));
                                enable = false;
                            } else {
                                int value = Integer.valueOf(s.toString());
                                if (rangeContent._end != RangeContent.NO_RANGE
                                        && (value < rangeContent._start
                                                || value > rangeContent._end)) {
                                    edit.setError(getActivity().getText(
                                            R.string.error_msg_out_of_range));
                                    enable = false;
                                } else {
                                    rangeContent._value = s.toString();
                                    rangeContent._label = s.toString();
                                }
                            }

                            // Update the ok button status.
                            MyDialog dialog = (MyDialog) getFragmentManager()
                                    .findFragmentByTag(DIALOG_TAG_EDIT);
                            Button ok = (Button) dialog.getDialog().findViewById(
                                    android.R.id.button1);
                            if (ok != null) {
                                ok.setEnabled(enable);
                            }
                        }
                    });

                    // Build the range view and set the range text.
                    TextView range = null;
                    if (rangeContent._end != RangeContent.NO_RANGE) {
                        range = new TextView(getActivity());
                        range.setGravity(Gravity.CENTER);
                        range.setTextAppearance(getActivity(),
                                android.R.style.TextAppearance_DeviceDefault_Small);
                        range.setText("(" + rangeContent._start + " - " + rangeContent._end + ")");
                    }

                    itemContentView.addView(rangeEditView, new LinearLayout.LayoutParams(
                            0, LayoutParams.WRAP_CONTENT, 1));
                    rangeEditView.addView(edit, new LinearLayout.LayoutParams(
                            LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
                    if (range != null) {
                        rangeEditView.addView(range, new LinearLayout.LayoutParams(
                                LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
                    }
                } else {
                    Log.e(TAG, "build view for dialog, the edit text's value must be the range.");
                }
            }
            return itemContentView;
        }
    }

    public interface OnSelectionChangedListener {
        public void onSelectionChanged(SelectionsContent allSelection, TestItem changedItem);
    }
}
