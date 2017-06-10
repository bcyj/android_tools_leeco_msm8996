/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.fcctest;

import android.app.ListFragment;
import android.content.Context;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.AdapterView.OnItemSelectedListener;

import com.qti.fcctest.Utils.SelectionsContent;
import com.qti.fcctest.Utils.ValueContent;

import java.util.ArrayList;
import java.util.HashMap;

public class TestContentFragment extends ListFragment {
    private static final String TAG = "TestContentFragment";

    private View mRootView;
    private LayoutInflater mInflater;

    private ListView mList;
    private MyArrayAdapter mAdapter;

    private int mTestId;
    private SelectionsContent mSelections;
    private OnSelectionChangedListener mListener = null;
    private String[] mTestItemsName;
    private HashMap<String, ArrayList<ValueContent>> mTestItemsList;

    private OnItemSelectedListener mValueChangedListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            int itemId = (Integer) parent.getTag();
            ValueContent value = ValueContent.getSpinnerContentValue((Spinner) parent, position);
            Log.d(TAG, "Value changed, item: " + mTestItemsName[itemId]
                    + ", new value: " + value._value);

            updateSelectionsContent(itemId, value);
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
            mTestItemsList = new HashMap<String, ArrayList<ValueContent>>();
        }

        mTestItemsList.clear();

        String[] items = getResources().getStringArray(R.array.tx_test_items);
        if (mTestId == Utils.ID_TX_TEST) {
            mTestItemsName = items;
        } else if (mTestId == Utils.ID_RX_TEST || mTestId == Utils.ID_SCW_TEST) {
            mTestItemsName = new String[] { items[0] };
        }

        // Get the items and options.
        if (mTestId == Utils.ID_TX_TEST || mTestId == Utils.ID_RX_TEST) {
            mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_CHANNEL],
                    getTestItemOptions(R.array.opt_channel, R.array.opt_channel_value));
        } else if (mTestId == Utils.ID_SCW_TEST) {
            mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_CHANNEL],
                    getTestItemOptions(R.array.opt_scw_channel, R.array.opt_scw_channel_value));
        }

        if (mTestId == Utils.ID_TX_TEST) {
            // For tx test, it need this two item.
            mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_RATE],
                    getTestItemOptions(R.array.opt_rate, R.array.opt_rate_value));
            mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_POWER],
                    getTestItemOptions(R.array.opt_power, R.array.opt_power_value));
            mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_POWER_MODE],
                    getTestItemOptions(R.array.opt_powermode, R.array.opt_powermode_value));
        }

        // Do not show these items
        /*
        mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_ANTENNA],
                getTestItemOptions(R.array.opt_antenna, 0));
        mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_TYPE],
                getTestItemOptions(R.array.opt_type, 0));
        mTestItemsList.put(mTestItemsName[Utils.ID_TEST_ITEM_PATTERN],
                getTestItemOptions(R.array.opt_pattern, 0));
        */

        // Set the adapter to the list.
        mAdapter = new MyArrayAdapter(this, getActivity(), R.layout.test_item, mTestItemsName);
        setListAdapter(mAdapter);
    }

    private ArrayList<ValueContent> getTestItemOptions(int labelsResId, int valuesResId) {
        ArrayList<ValueContent> valuesList = new ArrayList<ValueContent>();
        String[] labels = getResources().getStringArray(labelsResId);
        String[] values = null;
        if (valuesResId > 0) {
            values = getResources().getStringArray(valuesResId);
        }
        if (values != null && labels.length != values.length) {
            Log.e(TAG, "The label's length is not same as the values length. Please check it.");
            return valuesList;
        }

        for (int i = 0; i < labels.length; i++) {
            ValueContent value = new ValueContent(
                    values != null ? values[i] : labels[i],
                    labels[i]);
            valuesList.add(value);
        }
        return valuesList;
    }

    private void updateSelectionsContent(int itemId, ValueContent curValue) {
        if (curValue == null) return;

        // Update the selection.
        if (mSelections == null) {
            mSelections = new SelectionsContent(mTestId);
        }
        mSelections.setSelection(String.valueOf(itemId), curValue);

        // Notify the selection change.
        if (mListener != null) {
            mListener.onSelectionChanged(mSelections, itemId);
        }
    }

    private static class MyArrayAdapter extends ArrayAdapter<String> {
        private TestContentFragment fragment;

        public MyArrayAdapter(TestContentFragment fragment, Context context, int resource,
                String[] items) {
            super(context, resource, items);
            this.fragment = fragment;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View v = fragment.mInflater.inflate(R.layout.test_item, null);

            TextView testName = (TextView) v.findViewById(R.id.test_item_name);
            testName.setText(fragment.mTestItemsName[position]);

            Spinner testValue = (Spinner) v.findViewById(R.id.test_item_value);
            setSpinnerValues(position, testValue, null);
            return v;
        }

        private void setSpinnerValues(int position, Spinner spinnerView, String curValue) {
            if (spinnerView == null) return;

            // Set the tag and default value for the item.
            spinnerView.setTag(position);

            String itemName = fragment.mTestItemsName[position];
            ArrayList<ValueContent> values = fragment.mTestItemsList.get(itemName);
            if (TextUtils.isEmpty(curValue)) {
                curValue = values.get(0)._value;
            }

            ArrayAdapter<ValueContent> adapter = new ArrayAdapter<ValueContent>(
                    fragment.getActivity(), R.layout.spinner_item, values);
            spinnerView.setAdapter(adapter);
            ValueContent v = ValueContent.setSpinnerContentValue(spinnerView, curValue);
            fragment.updateSelectionsContent(position, v);
            spinnerView.setOnItemSelectedListener(fragment.mValueChangedListener);
        }

    }

    public interface OnSelectionChangedListener {
        public void onSelectionChanged(SelectionsContent allSelection, int changedItemId);
    }
}
