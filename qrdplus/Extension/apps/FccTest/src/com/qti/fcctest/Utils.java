/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.fcctest;

import android.text.TextUtils;
import android.util.Log;
import android.widget.Spinner;

import java.util.HashMap;

public class Utils {
    // The ID for the tx test and rx test.
    public static final int ID_TX_TEST = 0;
    public static final int ID_RX_TEST = 1;
    public static final int ID_SCW_TEST = 2;

    // This must be match the array list defined.
    public static final int ID_TEST_ITEM_CHANNEL = 0;
    public static final int ID_TEST_ITEM_RATE = 1;
    public static final int ID_TEST_ITEM_POWER = 2;
    public static final int ID_TEST_ITEM_POWER_MODE = 3;
    public static final int ID_TEST_ITEM_ANTENNA = 4;
    public static final int ID_TEST_ITEM_TYPE = 5;
    public static final int ID_TEST_ITEM_PATTERN = 6;

    /**
     * To save the spinner for constants values.
     */
    public static class ValueContent {
        private static final String TAG = "ValueContent";

        public final String _value;
        public final String _label;

        public ValueContent(String value, String label) {
            this._value = value;
            this._label = label;
        }

        public static ValueContent setSpinnerContentValue(Spinner spinner, String value) {
            for (int i = 0, count = spinner.getCount(); i < count; i++) {
                ValueContent sc = (ValueContent) spinner.getItemAtPosition(i);
                if (sc._value.equalsIgnoreCase(value)) {
                    spinner.setSelection(i, true);
                    Log.i(TAG, "Set selection for spinner(" + sc + ") with the value: " + value);
                    return sc;
                }
            }
            return null;
        }

        public static ValueContent getSpinnerContentValue(Spinner spinner, int position) {
            return (ValueContent) spinner.getItemAtPosition(position);
        }

        @Override
        public String toString() {
            return _label;
        }
    }

    public static class SelectionsContent {
        private int _testId;
        private HashMap<String, ValueContent> _values;

        public SelectionsContent(int testId) {
            _testId = testId;
            _values = new HashMap<String, ValueContent>();
        }

        public void reset(int testId) {
            _testId = testId;
            if (_values == null) {
                _values = new HashMap<String, ValueContent>();
            } else {
                _values.clear();
            }
        }

        public void setSelection(String itemId, ValueContent value) {
            if (TextUtils.isEmpty(itemId)) return;

            if (_values == null) {
                _values = new HashMap<String, ValueContent>();
            }
            _values.put(itemId, value);
        }

        public int getTestId() {
            return _testId;
        }

        public HashMap<String, ValueContent> getSelections() {
            return _values;
        }
    }

}
