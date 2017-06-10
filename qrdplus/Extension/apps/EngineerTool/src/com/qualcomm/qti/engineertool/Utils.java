/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.qualcomm.qti.engineertool.model.DialogModel;
import com.qualcomm.qti.engineertool.model.DoOpsModel;
import com.qualcomm.qti.engineertool.model.IntentModel;
import com.qualcomm.qti.engineertool.model.ListModel;
import com.qualcomm.qti.engineertool.model.OperationModel;
import com.qualcomm.qti.engineertool.model.TwoLineModel;
import com.qualcomm.qti.engineertool.model.IntentModel.Extra;

public class Utils {
    private static final String TAG = "Utils";

    public static class WaitingDialog extends DialogFragment {
        public static final int DISMISS_BY_USER = -1;
        public static final String TAG_LABEL = "waiting";

        private static final String ARG_MSG = "msg";
        private static final String ARG_MILLIS = "millis";

        private static final int MSG_DISMISS = 1;
        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == MSG_DISMISS) {
                    Dialog dialog = getDialog();
                    if (dialog != null) {
                        dialog.dismiss();
                    }
                }
                super.handleMessage(msg);
            }
        };

        public static WaitingDialog newInstance(Fragment target, int messageResId,
                int showInMillis) {
            WaitingDialog dialog = new WaitingDialog();
            dialog.setCancelable(false);

            Bundle bundle = new Bundle();
            bundle.putInt(ARG_MSG, messageResId);
            bundle.putInt(ARG_MILLIS, showInMillis);
            dialog.setArguments(bundle);

            if (target != null) dialog.setTargetFragment(target, 0);

            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final int msgResId = getArguments().getInt(ARG_MSG);
            ProgressDialog dialog = new ProgressDialog(getActivity());
            dialog.setMessage(getString(msgResId));
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void show(FragmentManager manager, String tag) {
            super.show(manager, tag);
            final int showMillis = getArguments().getInt(ARG_MILLIS);
            if (showMillis > 0) {
                mHandler.sendEmptyMessageDelayed(MSG_DISMISS, showMillis);
            }
        }

        @Override
        public void onDismiss(DialogInterface dialog) {
            super.onDismiss(dialog);
            Fragment target = getTargetFragment();
            if (target != null && target instanceof OnDialogDismissListener) {
                ((OnDialogDismissListener) target).onDialogDismiss();
            }
        }
    }

    public interface OnDialogDismissListener {
        public void onDialogDismiss();
    }

    public static void bindTwoLineView(Context context, View view, TwoLineModel data) {
        TextView labelView = (TextView) view.findViewById(R.id.label);
        TextView summaryView = (TextView) view.findViewById(R.id.summary);
        if (labelView == null || summaryView == null) return;

        String label = data.getLabel(context);
        if (TextUtils.isEmpty(label)) {
            // Shouldn't be here. Label should not as null.
            labelView.setVisibility(View.GONE);
            Log.e(TAG, "Label should not be null");
        } else {
            labelView.setVisibility(View.VISIBLE);
            labelView.setText(label);
        }

        String summary = data.getSummary(context);
        if (TextUtils.isEmpty(summary)) {
            // Shouldn't be here. Label should not as null.
            summaryView.setVisibility(View.GONE);
        } else {
            summaryView.setVisibility(View.VISIBLE);
            summaryView.setText(summary);
        }
    }

    public static boolean onClick(Activity activity, Fragment target, Parcelable targetData,
            Parcelable clickAction) {
        if (clickAction == null) {
            Log.w(TAG, "Do not found defined action, do nothing.");
            return false;
        }

        if (clickAction instanceof ListModel) {
            activity.startActivity(EngineerToolActivity.buildIntentWithContent(
                    activity.getApplicationContext(), clickAction));
            return true;
        } else if (clickAction instanceof DialogModel) {
            DialogFragmentBuilder builder =
                    DialogFragmentBuilder.newInstance(target, targetData,
                            (DialogModel) clickAction);
            builder.show(activity.getFragmentManager(), DialogFragmentBuilder.DIALOG_TAG);
            return true;
        } else if (clickAction instanceof IntentModel) {
            onClickAsIntentAction(activity, (IntentModel) clickAction);
            return true;
        } else if (clickAction instanceof DoOpsModel) {
            OperationModel[] ops = ((DoOpsModel) clickAction).getOperations();
            if (ops != null && ops.length > 0) {
                for (int i = 0; i < ops.length; i++) {
                    Operation op = Operation.getInstance(activity.getContentResolver());
                    op.doOperationAsync(ops[i].getFunction(), ops[i].getParams(), null);
                }
            }
        }

        Log.w(TAG, "Do not found matched action, do nothing.");
        return false;
    }

    private static void onClickAsIntentAction(Context context, IntentModel intentData) {
        Intent intent = new Intent();

        String action = intentData.getAction();
        if (!TextUtils.isEmpty(action)) {
            intent.setAction(action);
        }

        String packageName = intentData.getPackageName();
        if (!TextUtils.isEmpty(packageName)) {
            String className = intentData.getClassName();
            if (TextUtils.isEmpty(className)) {
                intent.setPackage(packageName);
            } else {
                intent.setClassName(packageName, className);
            }
        }

        Extra[] extras = intentData.getExtras();
        if (extras != null && extras.length > 0) {
            for (int i = 0; i < extras.length; i++) {
                Extra extra = extras[i];
                intent.putExtra(extra.getKey(), extra.getValue());
            }
        }

        switch (intentData.getType()) {
            case IntentModel.TYPE_ACTIVITY:
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
                break;
            case IntentModel.TYPE_SERVICE:
                context.startService(intent);
                break;
            case IntentModel.TYPE_BROADCAST:
                context.sendBroadcast(intent);
                break;
        }
    }
}
