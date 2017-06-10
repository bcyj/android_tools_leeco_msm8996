/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.app.Activity;
import android.app.Fragment;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.android.internal.app.LocalePicker;
import com.android.internal.app.LocalePicker.LocaleInfo;

import java.util.ArrayList;

public class TriggerWelcomeFragment extends Fragment implements OnClickListener {
    private static final String ARG_CARRIER_LIST = "carrier_list";
    private static final int REQUEST_START_TRIGGER = 1;

    private Spinner mSpinner;
    private int mCurrentIndex;
    private ArrayList<Carrier> mCarriers;

    public TriggerWelcomeFragment() {
    }

    public static TriggerWelcomeFragment newInstance(ArrayList<Carrier> carriers) {
        final TriggerWelcomeFragment fragment = new TriggerWelcomeFragment();

        final Bundle args = new Bundle(1);
        args.putParcelableArrayList(ARG_CARRIER_LIST, carriers);
        fragment.setArguments(args);

        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mCarriers = getArguments().getParcelableArrayList(ARG_CARRIER_LIST);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle bundle) {
        View rootView = inflater.inflate(R.layout.trigger_welcome_fragment, container, false);

        // Set the click listener for these two button.
        rootView.findViewById(R.id.start_trigger).setOnClickListener(this);
        rootView.findViewById(R.id.emergency_call).setOnClickListener(this);

        mSpinner = (Spinner) rootView.findViewById(R.id.language_spinner);

        ArrayList<LocaleInfo> list = new ArrayList<LocaleInfo>();
        mCurrentIndex = getLocaleInfos(LocalePicker.constructAdapter(getActivity()), list);
        mSpinner.setAdapter(
                new ArrayAdapter<LocaleInfo>(getActivity(), R.layout.spinner_item, list));
        mSpinner.setSelection(mCurrentIndex, true);

        return rootView;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == Activity.RESULT_OK
                && requestCode == REQUEST_START_TRIGGER) {
            getActivity().finish();
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_trigger:
                if (mSpinner.getSelectedItemPosition() != mCurrentIndex) {
                    LocaleInfo info = (LocaleInfo) mSpinner.getSelectedItem();
                    LocalePicker.updateLocale(info.getLocale());
                }
                Intent intent = new Intent();
                intent.setAction(Utils.ACTION_TRIGGER_START);
                intent.setClass(getActivity(), ConfigurationActivity.class);
                intent.putParcelableArrayListExtra(Utils.EXTRA_CARRIER_LIST, mCarriers);
                startActivityForResult(intent, REQUEST_START_TRIGGER);
                break;
            case R.id.emergency_call:
                // TODO: start the emergency call.
                break;
        }
    }

    private int getLocaleInfos(ArrayAdapter<LocaleInfo> from, ArrayList<LocaleInfo> to) {
        if (from == null || to == null) return 0;

        int selectedIndex = 0;
        for (int i = 0; i < from.getCount(); i++) {
            LocaleInfo info = from.getItem(i);
            to.add(i, info);
            if (info.getLocale().equals(getResources().getConfiguration().locale)) {
                selectedIndex = i;
            }
        }

        return selectedIndex;
    }

}
