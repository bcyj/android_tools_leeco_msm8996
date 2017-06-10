/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.bttest;

import android.content.Context;
import android.content.res.Resources;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Spinner;

import java.util.ArrayList;
import java.util.HashMap;

public class Utils {
    private static final String TAG = "Utils";

    // The test mode.
    public static final int TEST_MODE_NONE = 0;
    public static final int TEST_MODE_NO_CMD = 1;
    public static final int TEST_MODE_CMD = 2;

    // The ID for the tx test and rx test.
    public static final int ID_ONLY_BURST = 0;
    public static final int ID_CONTINUOUS = 1;
    public static final int ID_LOW_ENERGY = 2;

    // This must be match the array list defined.
    public static final int ID_TEST_ITEM_HOPPING = R.string.test_item_hopping;
    public static final int ID_TEST_ITEM_WHITENING = R.string.test_item_whitening;
    public static final int ID_TEST_ITEM_CHANNEL = R.string.test_item_channel;
    public static final int ID_TEST_ITEM_POWER = R.string.test_item_power;
    public static final int ID_TEST_ITEM_PACKET_TYPE = R.string.test_item_packet_type;
    public static final int ID_TEST_ITEM_TRANS_PATTERN = R.string.test_item_transmit_pattern;
    public static final int ID_TEST_ITEM_PAYLOAD_LENGTH = R.string.test_item_payload_length;
    public static final int ID_TEST_ITEM_TRANS_CHANNEL = R.string.test_item_transmit_channel;
    public static final int ID_TEST_ITEM_TEST_TYPE = R.string.test_item_test_type;
    public static final int ID_TEST_ITEM_PATTERN_LENGTH = R.string.test_item_pattern_length;
    public static final int ID_TEST_ITEM_TEST_PAYLOAD = R.string.test_item_test_payload;

    // The layout type for test item.
    public static final int LAYOUT_TYPE_SWITCH = 0;
    public static final int LAYOUT_TYPE_EDITTEXT = 1;
    public static final int LAYOUT_TYPE_SPINNER = 2;

    private static final String RANGES_SEP = ",";

    public static TestItem[] buildTestItems(Context context, int testID,
            HashMap<TestItem, ArrayList<ValueContent>> testList) {
        Log.i(TAG, "buildTestItems, testID = " + testID);
        Resources res = context.getResources();
        TestItem[] testItems = null;
        switch (testID) {
            case ID_ONLY_BURST:
                testItems = new TestItem[] {
                        new TestItem(LAYOUT_TYPE_SWITCH, ID_TEST_ITEM_HOPPING,
                                res.getString(ID_TEST_ITEM_HOPPING)),                   // 0
                        new TestItem(LAYOUT_TYPE_SWITCH, ID_TEST_ITEM_WHITENING,
                                res.getString(ID_TEST_ITEM_WHITENING)),                 // 1
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_CHANNEL,
                                res.getString(ID_TEST_ITEM_CHANNEL)),                   // 2
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_POWER,
                                res.getString(ID_TEST_ITEM_POWER)),                     // 3
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_PACKET_TYPE,
                                res.getString(ID_TEST_ITEM_PACKET_TYPE)),               // 4
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_TRANS_PATTERN,
                                res.getString(ID_TEST_ITEM_TRANS_PATTERN)),             // 5
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_PAYLOAD_LENGTH,
                                res.getString(ID_TEST_ITEM_PAYLOAD_LENGTH))             // 6
                };

                // 0 - hopping, true(default value)/false
                testList.put(testItems[0],
                        getTestItemOptions(context, R.array.opt_hopping, R.array.opt_hopping));

                // 1 - whitening, false(default value)/true
                testList.put(testItems[1],
                        getTestItemOptions(context, R.array.opt_whitening, R.array.opt_whitening));

                // 2 - channel
                // hopping = false, channel 0 - channel 78
                // hopping = true, channel 0 - channel 78, ... 5 items (default value)
                testList.put(testItems[2],
                        getRanges(context, 0, R.array.range_ob_channel_end_value, 0, 4));

                // 3 - power, 0 dBm - 9 dBm
                testList.put(testItems[3],
                        getTestItemOptions(context, R.string.power_x, 0, 9));

                // 4 - packet type, get the list from array
                testList.put(testItems[4],
                        getTestItemOptions(context, R.array.opt_packet_type, 0));

                // 5 - transmit pattern, get the list from array
                testList.put(testItems[5],
                        getTestItemOptions(context, R.array.opt_trans_pattern, 0));

                // 6 - payload length, the length range need match the selected packet type
                testList.put(testItems[6],
                        getRanges(context, 0, R.array.range_ob_payload_length_end_value, 0, 0));

                // Build the relation for the test items.
                // I)  hopping's value will be affect channel
                testItems[0].setRelationItem(testItems[2]);

                // II) packet type's value will be affect payload length
                testItems[4].setRelationItem(testItems[6]);

                break;
            case ID_CONTINUOUS:
                testItems = new TestItem[] {
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_TRANS_CHANNEL,
                                res.getString(ID_TEST_ITEM_TRANS_CHANNEL)),             // 0
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_POWER,
                                res.getString(ID_TEST_ITEM_POWER)),                     // 1
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_TEST_TYPE,
                                res.getString(ID_TEST_ITEM_TEST_TYPE)),                 // 2
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_PATTERN_LENGTH,
                                res.getString(ID_TEST_ITEM_PATTERN_LENGTH))             // 3
                };

                // 0 - transmit channel, channel 0 - channel 78
                testList.put(testItems[0], getRange(0, 78));

                // 1 - power, 0 dBm - 9 dBm
                testList.put(testItems[1],
                        getTestItemOptions(context, R.string.power_x, 0, 9));

                // 2 - test type, get the list from array
                testList.put(testItems[2],
                        getTestItemOptions(context, R.array.opt_test_type, 0));

                // 3 - pattern length, 0 - 31
                testList.put(testItems[3], getRange(0, 31));
                break;
            case ID_LOW_ENERGY:
                testItems = new TestItem[] {
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_CHANNEL,
                                res.getString(ID_TEST_ITEM_CHANNEL)),                   // 0
                        new TestItem(LAYOUT_TYPE_SPINNER, ID_TEST_ITEM_TEST_PAYLOAD,
                                res.getString(ID_TEST_ITEM_TEST_PAYLOAD)),              // 1
                        new TestItem(LAYOUT_TYPE_EDITTEXT, ID_TEST_ITEM_PAYLOAD_LENGTH,
                                res.getString(ID_TEST_ITEM_PAYLOAD_LENGTH))             // 2
                };

                // 0 - channel, channel 0 - channel 39
                testList.put(testItems[0], getRange(0, 39));

                // 1 - test payload, get the list from array
                testList.put(testItems[1],
                        getTestItemOptions(context, R.array.opt_test_payload, 0));

                // 2 - payload length, 0 - 37
                testList.put(testItems[2], getRange(0, 37));
                break;
        }

        return testItems;
    }

    // Build the test item for changed case.
    public static boolean updateTestItems(Context context, int testID,
            HashMap<TestItem, ArrayList<ValueContent>> testList, TestItem updateItem,
            ValueContent newValue) {
        if (testList == null || updateItem == null || newValue == null) return false;

        boolean update = false;
        // Update the value content for only burst channel.
        if (testID == ID_ONLY_BURST) {
            switch (updateItem._id) {
                case ID_TEST_ITEM_HOPPING:
                    if (Boolean.valueOf(newValue._value)) {
                        // hopping = true, channel 0 - channel 78, ... 5 items
                        testList.put(updateItem._relationItem,
                                getRanges(context, 0, R.array.range_ob_channel_end_value, 0, 4));
                    } else {
                        // hopping = false, channel 0 - channel 78
                        testList.put(updateItem._relationItem,
                                getRanges(context, 0, R.array.range_ob_channel_end_value, 0, 0));
                    }
                    update = true;
                    break;
                case ID_TEST_ITEM_PACKET_TYPE:
                    ArrayList<ValueContent> list = testList.get(updateItem);
                    int index = list.indexOf(newValue);
                    testList.put(updateItem._relationItem,
                            getRanges(context, 0, R.array.range_ob_payload_length_end_value,
                                    index, index));
                    update = true;
                    break;
            }
        }
        return update;
    }

    public static ValueContent buildValueFromRanges(boolean init, ArrayList<ValueContent> values) {
        StringBuilder value = new StringBuilder();
        for (int i = 0; i < values.size(); i++) {
            if (values.get(i) instanceof RangeContent) {
                if (i != 0) {
                    value.append(RANGES_SEP);
                }
                if (init) {
                    value.append(((RangeContent) values.get(i))._start);
                } else {
                    value.append(values.get(i)._value);
                }
            } else {
                Log.e(TAG, "build the value from the ranges, but the values is not the instance");
            }
        }
        return new ValueContent(value.toString(), value.toString());
    }

    public static String[] getValueArray(String valueContent) {
        if (TextUtils.isEmpty(valueContent)) return null;

        return valueContent.split(RANGES_SEP);
    }

    private static ArrayList<ValueContent> getTestItemOptions(Context context, int labelsResId,
            int start, int end) {
        ArrayList<ValueContent> valuesList = new ArrayList<ValueContent>();

        int length = end - start + 1;
        for (int i = start; i < length; i++) {
            ValueContent value = new ValueContent(
                    Integer.toString(i), context.getResources().getString(labelsResId, i));
            valuesList.add(value);
        }

        return valuesList;
    }

    private static ArrayList<ValueContent> getTestItemOptions(Context context, int labelsResId,
            int valuesResId) {
        ArrayList<ValueContent> valuesList = new ArrayList<ValueContent>();
        String[] labels = context.getResources().getStringArray(labelsResId);
        String[] values = null;
        if (valuesResId > 0) {
            values = context.getResources().getStringArray(valuesResId);
        }
        if (values != null && labels.length != values.length) {
            Log.e(TAG, "The label's length is not same as the values length. Please check it.");
            return valuesList;
        }

        for (int i = 0; i < labels.length; i++) {
            ValueContent value = new ValueContent(
                    values != null ? values[i] : String.valueOf(i),
                    labels[i]);
            valuesList.add(value);
        }
        return valuesList;
    }

    private static ArrayList<ValueContent> getRanges(Context context,
            int rangeStartResId, int rangeEndResId, int start, int end) {
        ArrayList<ValueContent> list = new ArrayList<ValueContent>();

        int[] rangStarts = null;
        if (rangeStartResId > 0) {
            rangStarts = context.getResources().getIntArray(rangeStartResId);
        }
        int[] rangeEnds = context.getResources().getIntArray(rangeEndResId);

        for (int i = start; i <= end; i++) {
            int rangeStart = rangeStartResId > 0 ? rangStarts[i] : 0;
            list.add(new RangeContent(rangeStart, rangeEnds[i]));
        }
        return list;
    }

    private static ArrayList<ValueContent> getRange(int start, int end) {
        ArrayList<ValueContent> list = new ArrayList<ValueContent>();
        list.add(new RangeContent(start, end));
        return list;
    }

    public static class TestItem implements Parcelable {
        public final int _layout_type;
        public final int _id;
        public final String _label;
        public TestItem _relationItem;

        public TestItem(Parcel in) {
            ClassLoader loader = getClass().getClassLoader();
            this._layout_type = in.readInt();
            this._id = in.readInt();
            this._label = in.readString();
            this._relationItem = in.readParcelable(loader);
        }

        public TestItem(int layoutType, int id, String label) {
            this._layout_type = layoutType;
            this._id = id;
            this._label = label;
        }

        public void setRelationItem(TestItem relationItem) {
            this._relationItem = relationItem;
        }

        @Override
        public String toString() {
            return _label;
        }

        public static final Parcelable.Creator<TestItem> CREATOR =
                new Parcelable.Creator<TestItem>() {
            public TestItem createFromParcel(Parcel in) {
                return new TestItem(in);
            }

            public TestItem[] newArray(int size) {
                return new TestItem[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(_layout_type);
            dest.writeInt(_id);
            dest.writeString(_label);
            dest.writeParcelable(_relationItem, 0);
        }
    }

    /**
     * To save the spinner for constants values.
     */
    public static class ValueContent implements Parcelable {
        private static final String TAG = "ValueContent";

        public String _value;
        public String _label;

        public ValueContent(Parcel in) {
            this._value = in.readString();
            this._label = in.readString();
        }

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

        public static final Parcelable.Creator<ValueContent> CREATOR =
                new Parcelable.Creator<ValueContent>() {
            public ValueContent createFromParcel(Parcel in) {
                return new ValueContent(in);
            }

            public ValueContent[] newArray(int size) {
                return new ValueContent[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(_value);
            dest.writeString(_label);
        }
    }

    public static class RangeContent extends ValueContent implements Parcelable {
        public static final int NO_RANGE = -1;

        public final int _start;
        public final int _end;

        public RangeContent(Parcel in) {
            super(in);
            this._start = in.readInt();
            this._end = in.readInt();
        }

        public RangeContent(int start, int end) {
            super("0", "0");
            this._start = start;
            this._end = end;
        }

        @Override
        public String toString() {
            return "range[" + _start + "," + _end + "]";
        }

        public static final Parcelable.Creator<RangeContent> CREATOR =
                new Parcelable.Creator<RangeContent>() {
            public RangeContent createFromParcel(Parcel in) {
                return new RangeContent(in);
            }

            public RangeContent[] newArray(int size) {
                return new RangeContent[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(_start);
            dest.writeInt(_end);
        }
    }

    public static class SelectionsContent {
        private int _testId;
        private HashMap<TestItem, ValueContent> _values;

        public SelectionsContent(int testId) {
            _testId = testId;
            _values = new HashMap<TestItem, ValueContent>();
        }

        public void reset(int testId) {
            _testId = testId;
            if (_values == null) {
                _values = new HashMap<TestItem, ValueContent>();
            } else {
                _values.clear();
            }
        }

        public void setSelection(TestItem item, ValueContent value) {
            if (item == null) return;

            if (_values == null) {
                _values = new HashMap<TestItem, ValueContent>();
            }
            _values.put(item, value);
        }

        public int getTestId() {
            return _testId;
        }

        public HashMap<TestItem, ValueContent> getSelections() {
            return _values;
        }

        public ValueContent getCurValue(TestItem key) {
            return _values.get(key);
        }
    }

}
