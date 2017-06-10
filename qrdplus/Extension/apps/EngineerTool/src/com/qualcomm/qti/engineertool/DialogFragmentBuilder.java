/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.engineertool.Utils.OnDialogDismissListener;
import com.qualcomm.qti.engineertool.Utils.WaitingDialog;
import com.qualcomm.qti.engineertool.model.CheckListItemModel;
import com.qualcomm.qti.engineertool.model.DialogModel;
import com.qualcomm.qti.engineertool.model.DialogModel.DialogButton;
import com.qualcomm.qti.engineertool.model.ViewModel.EditContent;
import com.qualcomm.qti.engineertool.model.ViewModel.EditModel;
import com.qualcomm.qti.engineertool.model.ViewModel;
import com.qualcomm.qti.engineertool.model.ViewModel.SpinnerModel;
import com.qualcomm.qti.engineertool.model.ViewModel.SpinnerModel.SpinnerContent;
import com.qualcomm.qti.engineertool.model.ViewModel.SwitchModel;

public class DialogFragmentBuilder extends DialogFragment {
    private static final String TAG = "DialogFragmentBuilder";

    public static final String DIALOG_TAG = "dialog";

    private static final String ARG_TARGET_DATA = "target_data";
    private static final String ARG_DIALOG_DATA = "dialog_data";

    public static DialogFragmentBuilder newInstance(Fragment target, Parcelable targetData,
            DialogModel dialogData) {
        DialogFragmentBuilder dialog = new DialogFragmentBuilder();

        final Bundle args = new Bundle(1);
        args.putParcelable(ARG_TARGET_DATA, targetData);
        args.putParcelable(ARG_DIALOG_DATA, dialogData);
        dialog.setArguments(args);
        if (target != null) dialog.setTargetFragment(target, 0);

        return dialog;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Parcelable targetData = getArguments().getParcelable(ARG_TARGET_DATA);
        final DialogModel dialogData = getArguments().getParcelable(ARG_DIALOG_DATA);
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

        // Set dialog's icon.
        int iconResId = dialogData.getIconResId();
        if (iconResId > 0) {
            builder.setIcon(iconResId);
        }

        // Set dialog's title & message.
        String title = dialogData.getTitle(getActivity());
        if (!TextUtils.isEmpty(title)) {
            builder.setTitle(title);
        }

        String message = dialogData.getMessage(getActivity());
        if (!TextUtils.isEmpty(message)) {
            builder.setMessage(message);
        }

        final ViewModel viewData = dialogData.getView();
        if (viewData != null) {
            DialogViewBuilder viewBuilder = new DialogViewBuilder(getActivity(), viewData);
            builder.setView(viewBuilder.getView());
        }

        final DialogButton positiveBtn = dialogData.getPositiveButton();
        if (positiveBtn != null) {
            builder.setPositiveButton(positiveBtn.getLabel(getActivity()), new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    DialogButtonAction action = new DialogButtonAction(getActivity(),
                            getTargetFragment(), targetData, positiveBtn,
                            viewData == null ? null : viewData.getContent());
                    action.onClick();
                }
            });
        }

        final DialogButton neutralBtn = dialogData.getNeutralButton();
        if (neutralBtn != null) {
            builder.setNeutralButton(neutralBtn.getLabel(getActivity()), new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    DialogButtonAction action = new DialogButtonAction(getActivity(),
                            getTargetFragment(), targetData, neutralBtn,
                            viewData == null ? null : viewData.getContent());
                    action.onClick();
                }
            });
        }

        final DialogButton negativeBtn = dialogData.getNegativeButton();
        if (negativeBtn != null) {
            builder.setNegativeButton(negativeBtn.getLabel(getActivity()), new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    DialogButtonAction action = new DialogButtonAction(getActivity(),
                            getTargetFragment(), targetData, negativeBtn,
                            viewData == null ? null : viewData.getContent());
                    action.onClick();
                }
            });
        }

        return builder.create();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        final DialogModel dialogData = getArguments().getParcelable(ARG_DIALOG_DATA);
        ViewModel viewData = dialogData != null ? dialogData.getView() : null;
        EditContent content = viewData != null ? viewData.getContent() : null;
        if (content != null) {
            content.resetValues();
        }

        Fragment target = getTargetFragment();
        if (target != null && target instanceof OnDialogDismissListener) {
            ((OnDialogDismissListener) target).onDialogDismiss();
        }
    }

    private static class DialogViewBuilder {
        private Activity mActivity;
        private ViewModel mViewData;

        public DialogViewBuilder(Activity activity, ViewModel viewData) {
            mActivity = activity;
            mViewData = viewData;
        }

        public View getView() {
            View view = mActivity.getLayoutInflater().inflate(R.layout.dialog_view, null);

            TextView infoTop = (TextView) view.findViewById(R.id.dialog_view_text_top);
            TextView infoBottom = (TextView) view.findViewById(R.id.dialog_view_text_bottom);

            String info = mViewData.getInfo(mActivity);
            if (!TextUtils.isEmpty(info)) {
                switch (mViewData.getInfoGravityType()) {
                    case ViewModel.INFO_GRAVITY_TOP:
                        infoTop.setVisibility(View.VISIBLE);
                        infoBottom.setVisibility(View.GONE);
                        infoTop.setText(info);
                        break;
                    case ViewModel.INFO_GRAVITY_BOTTOM:
                        infoTop.setVisibility(View.GONE);
                        infoBottom.setVisibility(View.VISIBLE);
                        infoBottom.setText(info);
                }
            } else {
                infoTop.setVisibility(View.GONE);
                infoBottom.setVisibility(View.GONE);
            }

            EditText edit = (EditText) view.findViewById(R.id.dialog_view_edit);
            View switchContainer = view.findViewById(R.id.dialog_view_switch_container);
            Spinner spinner = (Spinner) view.findViewById(R.id.dialog_view_spinner);

            EditContent content = mViewData.getContent();
            if (content != null) {
                switch (mViewData.getContentType()) {
                    case ViewModel.TYPE_EDIT:
                        edit.setVisibility(View.VISIBLE);
                        switchContainer.setVisibility(View.GONE);
                        spinner.setVisibility(View.GONE);

                        bindEditView(edit, (EditModel) content);
                        break;
                    case ViewModel.TYPE_SWITCH:
                        switchContainer.setVisibility(View.VISIBLE);
                        edit.setVisibility(View.GONE);
                        spinner.setVisibility(View.GONE);

                        bindSwitchView(switchContainer, (SwitchModel) content);
                        break;
                    case ViewModel.TYPE_SPINNER:
                        spinner.setVisibility(View.VISIBLE);
                        edit.setVisibility(View.GONE);
                        switchContainer.setVisibility(View.GONE);

                        bindSpinnerView(spinner, (SpinnerModel) content);
                        break;
                }
            } else {
                edit.setVisibility(View.GONE);
                switchContainer.setVisibility(View.GONE);
                spinner.setVisibility(View.GONE);
            }
            return view;
        }

        private void bindEditView(final EditText editView, final EditModel editData) {
            if (editView == null || editData == null) return;

            String hint = editData.getHint(mActivity);
            if (!TextUtils.isEmpty(hint)) {
                editView.setHint(hint);
            }

            // Get the curValue and set to views.
            Operation op = Operation.getInstance(mActivity.getContentResolver());
            String curValue = op.doOperation(editData.getFunction(), editData.getParams());
            editData.setCurValue(curValue);
            editView.setText(curValue);
            if (!TextUtils.isEmpty(curValue)) {
                editView.setSelection(editView.getText().length());
            }

            editView.addTextChangedListener(new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    // Do nothing.
                }
                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                    // Do nothing.
                }
                @Override
                public void afterTextChanged(Editable s) {
                    String newValue = editView.getText().toString();
                    editData.setNewValue(newValue);
                }
            });
        }

        private void bindSwitchView(final View containerView, final SwitchModel switchData) {
            if (containerView == null || switchData == null) return;

            final TextView textView =
                    (TextView) containerView.findViewById(R.id.dialog_view_switch_text);
            final Switch switchView =
                    (Switch) containerView.findViewById(R.id.dialog_view_switch);

            String label = switchData.getLabel(mActivity);
            if (!TextUtils.isEmpty(label)) {
                textView.setText(label);
            }

            Operation op = Operation.getInstance(mActivity.getContentResolver());
            String curValue = op.doOperation(switchData.getFunction(), switchData.getParams());
            switchData.setCurValue(curValue);
            boolean checked = switchData.getChecked(curValue);
            switchView.setChecked(checked);

            switchView.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    switchData.setNewValue(isChecked);
                }
            });
        }

        private void bindSpinnerView(final Spinner spinnerView, final SpinnerModel spinnerData) {
            if (spinnerView == null || spinnerData == null) return;

            ArrayAdapter<SpinnerContent> adapter = new ArrayAdapter<SpinnerContent>(
                    mActivity, R.layout.spinner_item, spinnerData.getSpinnerContents()) {
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    View view;
                    if (convertView == null) {
                        view = mActivity.getLayoutInflater().inflate(
                                R.layout.spinner_item, parent, false);
                    } else {
                        view = convertView;
                    }
                    TextView text = (TextView) view.findViewById(android.R.id.text1);
                    text.setText(getItem(position).getLabel(mActivity));

                    return view;
                }

                @Override
                public View getDropDownView(int position, View convertView, ViewGroup parent) {
                    return getView(position, convertView, parent);
                }
            };
            spinnerView.setAdapter(adapter);

            Operation op = Operation.getInstance(mActivity.getContentResolver());
            String curValue = op.doOperation(spinnerData.getFunction(), spinnerData.getParams());
            spinnerData.setCurValue(curValue);
            setSpinnerContentValue(spinnerView, curValue);

            spinnerView.setOnItemSelectedListener(new OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position,
                        long id) {
                    SpinnerContent content = getSpinnerContentValue((Spinner) parent, position);
                    spinnerData.setNewValue(content.getValue());
                }
                @Override
                public void onNothingSelected(AdapterView<?> parent) {
                }
            });
        }

        private SpinnerContent setSpinnerContentValue(Spinner spinner, String value) {
            if (TextUtils.isEmpty(value)) {
                spinner.setSelection(0, true);
                return (SpinnerContent) spinner.getItemAtPosition(0);
            }

            for (int i = 0, count = spinner.getCount(); i < count; i++) {
                SpinnerContent sc = (SpinnerContent) spinner.getItemAtPosition(i);
                if (sc.getValue().equalsIgnoreCase(value)) {
                    spinner.setSelection(i, true);
                    return sc;
                }
            }

            return null;
        }

        private SpinnerContent getSpinnerContentValue(Spinner spinner, int position) {
            return (SpinnerContent) spinner.getItemAtPosition(position);
        }
    }

    private class DialogButtonAction {
        private Activity mActivity;
        private Fragment mTargetFragment;
        private Parcelable mTargetData;
        private DialogButton mButtonData;
        private EditContent mEditContent;

        public DialogButtonAction(Activity activity, Fragment target, Parcelable targetData,
                DialogButton buttonData, EditContent editContent) {
            mActivity = activity;
            mTargetFragment = target;
            mTargetData = targetData;
            mButtonData = buttonData;
            mEditContent = editContent;
        }

        public void onClick() {
            Parcelable clickAction = mButtonData.getClickAction();
            if (clickAction == null
                    && mEditContent != null
                    && mEditContent.getNewValue() != null
                    && !TextUtils.isEmpty(mButtonData.getFunction())) {
                Operation op = Operation.getInstance(mActivity.getContentResolver());
                String newParams = Operation.buildParams(mButtonData.getParams(),
                        mEditContent.getNewValue());
                String result = op.doOperation(mButtonData.getFunction(), newParams);
                if (Operation.EMPTY.equals(result)) {
                    // Failed, show the toast.
                    Toast.makeText(mActivity, R.string.set_value_failed, Toast.LENGTH_LONG).show();
                } else {
                    Log.i(TAG, "Do operation successfully.");
                    if (mTargetData != null && mTargetData instanceof CheckListItemModel) {
                        CheckListItemModel data = (CheckListItemModel) mTargetData;
                        data.setUpdateStatus(true);
                        data.setCurrentResult(mEditContent.getNewValue());
                    }

                    // Check if set the delay value, if set, we need display the waiting dialog.
                    int delayMillis = mButtonData.getDelayMillis();
                    if (delayMillis > 0) {
                        WaitingDialog waitingDialog = WaitingDialog.newInstance(mTargetFragment,
                                R.string.update_value, delayMillis);
                        waitingDialog.show(getFragmentManager(), WaitingDialog.TAG_LABEL);
                    }
                }
            } else if (clickAction != null) {
                Utils.onClick(mActivity, null, null, clickAction);
            }
        }
    }
}
