/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Fragment;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ExpandableListView;
import android.widget.ExpandableListView.OnChildClickListener;
import android.widget.ListView;

import com.qualcomm.qti.engineertool.Utils.OnDialogDismissListener;
import com.qualcomm.qti.engineertool.Utils.WaitingDialog;
import com.qualcomm.qti.engineertool.model.CheckListItemModel;
import com.qualcomm.qti.engineertool.model.GroupModel;
import com.qualcomm.qti.engineertool.model.ListModel;

public class EngineerToolFragment extends Fragment implements OnDialogDismissListener {
    private static final String TAG = "EngineerToolFragment";

    private static final String ARG_DATA = "data";

    private ListModel mData;
    private ListView mListView = null;

    public static EngineerToolFragment newInstance(Parcelable data) {
        final EngineerToolFragment fragment = new EngineerToolFragment();

        final Bundle args = new Bundle(1);
        args.putParcelable(ARG_DATA, data);
        fragment.setArguments(args);

        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Parcelable data = getArguments().getParcelable(ARG_DATA);
        if (data == null || !(data instanceof ListModel)) {
            // TODO: Shouldn't be here. Display the error message.
        }
        mData = (ListModel) data;
        String title = mData.getTitle(getActivity());
        if (!TextUtils.isEmpty(title)) {
            getActivity().getActionBar().setTitle(title);
        }

        setHasOptionsMenu(mData.getMenuType() != ListModel.MENU_NONE);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle bundle) {
        View rootView = inflater.inflate(R.layout.fragment, container, false);

        ExpandableListView groupList = (ExpandableListView) rootView.findViewById(R.id.group_list);
        ListView checkList = (ListView) rootView.findViewById(R.id.list);

        if (mData.getListViewType() == ListModel.TYPE_GROUP_LIST) {
            groupList.setVisibility(View.VISIBLE);
            checkList.setVisibility(View.GONE);

            final GroupListAdapter adapter = new GroupListAdapter(getActivity(),
                    (GroupModel[]) mData.getList());
            groupList.setAdapter(adapter);
            groupList.setOnChildClickListener(new OnChildClickListener() {
                @Override
                public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
                        int childPosition, long id) {
                    return adapter.onChildClick(parent, v, groupPosition, childPosition, id);
                }
            });
            mListView = groupList;
        } else if (mData.getListViewType() == ListModel.TYPE_CHECK_LIST) {
            groupList.setVisibility(View.GONE);
            checkList.setVisibility(View.VISIBLE);

            final CheckListAdapter adapter = new CheckListAdapter(getActivity(),
                    (CheckListItemModel[]) mData.getList());
            checkList.setAdapter(adapter);
            checkList.setOnItemClickListener(new OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    adapter.onItemClick(EngineerToolFragment.this, parent, view, position, id);
                }
            });
            mListView = checkList;
        } else {
            Log.e(TAG, "Do not support now. Please check the xml.");
        }

        return rootView;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.list_activity_menu, menu);

        int menuType = mData.getMenuType();
        MenuItem load = menu.findItem(R.id.load);
        if (load != null) load.setVisible((menuType & ListModel.MENU_LOAD) != 0);

        MenuItem apply = menu.findItem(R.id.apply);
        if (apply != null) apply.setVisible((menuType & ListModel.MENU_APPLY) != 0);
    }

    @Override
    public void onDialogDismiss() {
        WaitingDialog waitDialog = (WaitingDialog) getFragmentManager().findFragmentByTag(
                WaitingDialog.TAG_LABEL);
        if (waitDialog == null
                || waitDialog.getDialog() == null
                || !waitDialog.getDialog().isShowing()) {
            if (mListView != null) {
                mListView.invalidateViews();
            }
        }
    }

}
