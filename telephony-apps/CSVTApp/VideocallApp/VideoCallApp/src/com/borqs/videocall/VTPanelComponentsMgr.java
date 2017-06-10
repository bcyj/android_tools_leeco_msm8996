/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.text.InputType;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.LinearLayout;
import java.lang.Thread;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.nio.ByteBuffer;
import android.os.SystemProperties;
import android.view.Gravity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.widget.Toast;
//import android.text.method.DialerKeyListener;
import android.text.TextWatcher;
import android.text.Editable;
import android.text.method.NumberKeyListener;
import android.media.AudioManager;

public class VTPanelComponentsMgr implements VTPanel.IPanelManager {

    static final String TAG = "VTPanelComponentsMgr";
    private static final boolean DBG = true;
    private VideoCallScreen mScreen;
    private VideoCallApp mApp;

    private static final long sleepTime = 1000;

    // FIXME: move to VTPanel manager
    public static final int VTPANEL_EVENT_BASE = 0;
    public static final int VTPANEL_EVENT_INFLATE_DONE = VTPANEL_EVENT_BASE + 1;
    public static final int VTPANEL_EVENT_ANIMATION_START = VTPANEL_EVENT_BASE + 2;
    public static final int VTPANEL_EVENT_SHOW_KEYPAD = VTPANEL_EVENT_BASE + 3;
    public static final int VTPANEL_EVENT_SHOW_ACTION_BUTTON = VTPANEL_EVENT_BASE + 4;
    public static final int VTPANEL_EVENT_ANIMATION_DONE = VTPANEL_EVENT_BASE + 5;

    private static final int ARROW_LOCAO_2_REMOTE = 0;
    private static final int ARROR_REMOTE_2_LOCAL = 1;

    private static final int MSG_BASE = 3000;
    private static final int MSG_DISMISS_DIALOG = MSG_BASE + 1;
    // vtpanel
    private VTPanel mMainPanel;
    private TextWatcher inputWatcher = new TextWatcher() {
        public void afterTextChanged(Editable s) {
        }

        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (start >= 0 && count > 0) {
                String editTextString = mUserInputView.getText().toString();
                // mUserKeyPadInputView.append(s);
                if (MyLog.DEBUG)
                    MyLog.v(TAG, "begin to send to native last of" + editTextString);
                if (editTextString != null && mApp != null) {
                    try {
                        mApp.mVTService.sendUserInput(editTextString.substring(editTextString
                                .length() - 1));
                    } catch (Exception e) {
                        if (MyLog.DEBUG)
                            Log.e(TAG, "error when send user input: " + e);
                    }
                }
            }
        }
    };
    // components of VTPanel
    VTPanelCallCard mCallCard;
    VTPanelLocalWindow mLocalWin;
    VTPanelRemoteWindow mRemoteWin;
    VTPanelControlButtons mCtrlBtns;
    VTPanelActionButtons mActionBtns;
    VTPanelFallBackPanel mFallBackPanel;

    // subcomponents of VTPanel
    private ImageView mArrowLocal2Remote;
    private ImageView mArrowRemote2Local;

    // TODO: record video should be added by next version
    // private View mBtnRecordVideo;

    public ImageView cameraImg;
    private View mBtnCapturePhoto;
    private View mBtnOpenCloseCamera;
    // private View mBtnSwitchCamera;
    private View mBtnSetCamera;
    private View mBtnSwitchSpeaker;
    // private View mBtnRemoteOnly;
    // private View mBtnRemoteWindow;
   // private ImageView mImgSwapWindow;

    private TextView mNameView;
    private TextView mPhoneNumberView;
    private EditText mUserInputView;
    private TextView mCardTitleView;
    // private TextView mUserKeyPadInputView;

    private View mBtnCallingEndCall;
    private View mBtnEndCall;
    private View mBtnCloseKeyboard;
    private View mBtnOpenKeyboard;

    private View mBtnContast;
    private View mBtnBright;
    private View mBtnScreenBright;
    private ImageView mRepImage;
    private ImageView toSpeaker;
    private TextView txtSpeaker;
    private SeekBar mSkBarContast;
    private SeekBar mSkBarBright;
    private SeekBar mSkBarScreenBright;
 //   private Button mBtnBack;
    private LinearLayout mLayoutSkBarContast;
    private LinearLayout mLayoutSkBarBright;

    private ProgressDialog progressDlg;
    private View focusButton = null;
    private boolean isProgressing = false;
    private boolean isRemoteOnly = false;
    private Dialog setCameraDlg;

    // for remote surface
    private static PhoneKeyListener sInstance;

    static class PhoneKeyListener extends NumberKeyListener {

        private PhoneKeyListener() {
            super();
        }

        protected char[] getAcceptedChars() {
            return CHARACTERS;
        }

        public static PhoneKeyListener getInstance() {
            if (sInstance != null)
                return sInstance;

            sInstance = new PhoneKeyListener();
            return sInstance;
        }

        public int getInputType() {
            return InputType.TYPE_NULL;
        }

        /**
         * The characters that are used.
         *
         * @see KeyEvent#getMatch
         * @see #getAcceptedChars
         */
        public static final char[] CHARACTERS = new char[] { '0', '1', '2', '3', '4', '5', '6',
                '7', '8', '9', '#', '*' };
    }

    class SetCameraDialog extends Dialog {
        private Handler mUpdateCameraParamHandler;
        private Looper mUpdateCameraParamLooper;

        public SetCameraDialog(Context context) {
            super(context);
        }

        protected void onCreate(Bundle savedInstanceState) {
            if (MyLog.DEBUG)
                MyLog.d("SetCameraDialog", "onCreate");
            super.onCreate(savedInstanceState);
            getWindow().requestFeature(Window.FEATURE_NO_TITLE);
            // WindowManager.LayoutParams wl = getWindow().getAttributes();
            // wl.alpha=0.5f;
            // getWindow().setAttributes(wl);
            setContentView(R.layout.dialog_set_camera);
            setCanceledOnTouchOutside(true);
            mBtnContast = this.findViewById(R.id.dialog_btn_contrast);
            mBtnBright = this.findViewById(R.id.dialog_btn_bright);
            mBtnScreenBright = this.findViewById(R.id.btn_screen_bright);
            mSkBarContast = (SeekBar) this.findViewById(R.id.dialog_contrast_val_seek);
            mSkBarBright = (SeekBar) this.findViewById(R.id.dialog_bright_val_seek);
            mSkBarScreenBright = (SeekBar) this.findViewById(R.id.screen_bright_val_seek);
          //  mBtnBack = (Button) this.findViewById(R.id.dialog_btn_back);
            mLayoutSkBarContast = (LinearLayout) this.findViewById(R.id.dialog_contrast_ctl_group);
            mLayoutSkBarBright = (LinearLayout) this.findViewById(R.id.dialog_bright_ctl_group);

            if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_SECONDARY) {
                mLayoutSkBarContast.setVisibility(View.GONE);
                mLayoutSkBarBright.setVisibility(View.GONE);
            } else {
                HandlerThread updateCameraParamHandlerThread = new HandlerThread(
                        "UpdateCameraParam");
                updateCameraParamHandlerThread.start();
                mUpdateCameraParamLooper = updateCameraParamHandlerThread.getLooper();
                mUpdateCameraParamHandler = new Handler(mUpdateCameraParamLooper) {
                    public void handleMessage(Message msg) {
                        switch (msg.what) {
                        case CameraSetting.BRIGHT_SETTING:
                            // Go through
                        case CameraSetting.CONSTRAST_SETTING:
                            if (MyLog.DEBUG) {
                                MyLog.d("SetCameraDialog", "handle Param:" + msg.what
                                        + "; progress:" + (Integer) msg.obj);
                            }
                            mApp.mCameraSetting.updateCameraParams(mApp.mVTService, msg.what,
                                    (Integer) msg.obj);
                            if (MyLog.DEBUG) {
                                MyLog.d("SetCameraDialog", "update done");
                            }
                            break;
                        default:
                            if (MyLog.DEBUG) {
                                MyLog.d("SetCameraDialog", "handle unknow msg:" + msg);
                            }
                            break;
                        }
                    };
                };
            }
            mSkBarContast.setMax(mApp.mCameraSetting
                    .getSettingDegree(CameraSetting.CONSTRAST_SETTING));
            mSkBarContast.setProgress(mApp.mCameraSetting
                    .getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
            mSkBarBright.setMax(mApp.mCameraSetting.getSettingDegree(CameraSetting.BRIGHT_SETTING));
            mSkBarBright.setProgress(mApp.mCameraSetting
                    .getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
            mSkBarScreenBright.setMax(VTCallUtils.MAX_BACKLIGHT - VTCallUtils.MIN_BACKLIGHT);
            // mSkBarScreenBright.setProgress( (VTCallUtils.MAX_BACKLIGHT -
            // VTCallUtils.MIN_BACKLIGHT) / 2);
            mSkBarScreenBright.setProgress(mApp.mScreenBrightness);
            // get seek value of seek bar
            mSkBarBright.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromTouch) {
                    Message msg = mUpdateCameraParamHandler
                            .obtainMessage(CameraSetting.BRIGHT_SETTING);
                    msg.obj = progress;
                    mUpdateCameraParamHandler.sendMessage(msg);
                }

                public void onStartTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }

                public void onStopTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }
            });

            mSkBarContast.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromTouch) {
                    /*
                     * For now, only back camera can adjust contrast, in future
                     * if the front camera supports to adjust contrast and that
                     * causes crash too, it will need to pauseCameraPreview()
                     */
                    if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_MAIN) {
                        /*
                         * Stop updating the preview surface, to fix the bug
                         * crash because of adjusting contrast in VT call,
                         */
                        mApp.mVTService.pauseCameraPreview(true, VTManager.VideoSource.CAMERA_MAIN);
                        Message msg = mUpdateCameraParamHandler
                                .obtainMessage(CameraSetting.CONSTRAST_SETTING);
                        msg.obj = progress;
                        mUpdateCameraParamHandler.sendMessage(msg);
                        /*
                         * Start updating the preview surface after adjusting
                         * contrast
                         */
                        mApp.mVTService
                                .pauseCameraPreview(false, VTManager.VideoSource.CAMERA_MAIN);
                    } else if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_SECONDARY) {
                        Message msg = mUpdateCameraParamHandler
                                .obtainMessage(CameraSetting.CONSTRAST_SETTING);
                        msg.obj = progress;
                        mUpdateCameraParamHandler.sendMessage(msg);
                    }
                }

                public void onStartTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }

                public void onStopTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }
            });

            // get seek value of seek bar
            mSkBarScreenBright.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromTouch) {
                    // TODO Auto-generated method stub
                    boolean ret = true;
                    if (mApp.mScreenBrightness == progress)
                        return;

                    ret = VTCallUtils.setScreenBrightness(progress + VTCallUtils.MIN_BACKLIGHT);
                    if (ret) {
                        mApp.mScreenBrightness = progress;
                    } else {
                        mSkBarScreenBright.setProgress(mApp.mScreenBrightness);
                    }
                }

                public void onStartTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }

                public void onStopTrackingTouch(SeekBar seekBar) {
                    // TODO Auto-generated method stub
                }
            });
        }

        protected void onStart() {
            if (MyLog.DEBUG)
                MyLog.d("SetCameraDialog", "onStart");
            mSkBarContast.setMax(mApp.mCameraSetting
                    .getSettingDegree(CameraSetting.CONSTRAST_SETTING));
            mSkBarBright.setMax(mApp.mCameraSetting.getSettingDegree(CameraSetting.BRIGHT_SETTING));
            mSkBarContast.setProgress(mApp.mCameraSetting
                    .getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
            mSkBarBright.setProgress(mApp.mCameraSetting
                    .getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
        }

        protected void onStop() {
            if (MyLog.DEBUG)
                MyLog.d("SetCameraDialog", "+++++++++++++++++++++++++++");
            if (null != mUpdateCameraParamLooper) {
                mUpdateCameraParamLooper.quit();
            }
        }
    }

    class SwitchCameraTask implements Runnable {
        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "swtich camera in the single thread.");
            onSwitchCamera();
            /*
             * if (mApp.getVTServiceInterface().isUseVTCamera()) { // The new
             * VTCamera switch camera is so far, we have to sleep 1 Sec. try {
             * Thread.sleep(sleepTime); }catch( Exception e){ if (MyLog.DEBUG)
             * MyLog.d(TAG, e.getMessage()); } }
             */
            mMessageListener.sendEmptyMessage(MSG_DISMISS_DIALOG);
        }
    }

    class OpenCloseCameraTask implements Runnable {
        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Open or close camera in the single thread.");
            mApp.openCloseCamera();
            // TODO Auto-generated method stub
            mMessageListener.sendEmptyMessage(MSG_DISMISS_DIALOG);
        }

    }

    class CloseLocalVideoTask implements Runnable {
        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "close local video in the single thread.");
            if (!mApp.getVTServiceInterface().isUseVTCamera()) {
                if (mApp.mCurrentVideoSource != VTManager.VideoSource.CAMERA_NONE) {
                    mApp.mVTService.setVideoSource(VTManager.VideoSource.CAMERA_NONE,
                            mApp.mStrReplaceImagePath, null);
                    if (mApp.mStrReplaceImagePath == null) {
                        mApp.mVTService.sendUserInput(VideoCallApp.MSG_CLOSE_CAMERA);
                    }
                }

            }
            // TODO Auto-generated method stub
            mMessageListener.sendEmptyMessage(MSG_DISMISS_DIALOG);
        }

    }

    private Handler mMessageListener = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_DISMISS_DIALOG:
                progressDlg.dismiss();
                if (focusButton == mBtnOpenCloseCamera) {
                    mMainPanel.cancelDelayedSwitchMode();
                    mBtnOpenCloseCamera.setBackgroundResource(0);
                }
                // else if (focusButton == mBtnSwitchCamera) {
                // mMainPanel.cancelDelayedSwitchMode();
                // mBtnSwitchCamera.setBackgroundResource(0);
                // }
                // else if (focusButton == mBtnRemoteOnly) {
                // if (isRemoteOnly) {
                // mBtnRemoteOnly.setBackgroundResource(0);
                // mScreen.setLocalSurfaceVisibility(View.GONE);
                // switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE_ONLY);
                // } else {
                // mBtnRemoteOnly.setBackgroundResource(0);
                // switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
                // }
                // }
                isProgressing = false;
                focusButton = null;
                break;

            }
        }
    };

    VTPanelComponentsMgr(VideoCallScreen screen, VideoCallApp app) {
        mScreen = screen;
        mApp = app;
        initPanel();
        initComponents();
        initSubComponents();
    }

    ProgressDialog getProgressDialog() {
        return progressDlg;
    }

    View getCallCard() {
        return mCallCard;
    }

    View getRemoteSurface() {
        return mRemoteWin;
    }

    View getLocalSurface() {
        return mLocalWin;
    }

    View getControlButtons() {
        return mCtrlBtns;
    }

    private void initPanel() {
        log("initPanel...");
        mMainPanel = (VTPanel) mScreen.findViewById(R.id.vt_panel);
        mMainPanel.setPanelManager(this);
        mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);
    }

    private boolean initComponents() {
        mCallCard = (VTPanelCallCard) mScreen.findViewById(R.id.vtpanel_call_card);
        if (mCallCard == null) {
            throw new IllegalArgumentException("call card does not exist");
        }

        mLocalWin = (VTPanelLocalWindow) mScreen.findViewById(R.id.vtpanel_local_win);
        if (mLocalWin == null) {
            throw new IllegalArgumentException("local window does not exist");
        }

        mRemoteWin = (VTPanelRemoteWindow) mScreen.findViewById(R.id.vtpanel_remote_win);
        if (mRemoteWin == null) {
            throw new IllegalArgumentException("remote window does not exist");
        }

        mCtrlBtns = (VTPanelControlButtons) mScreen.findViewById(R.id.vtpanel_control_buttons);
        if (mCtrlBtns == null) {
            throw new IllegalArgumentException("control buttons do not exist");
        }

        mActionBtns = (VTPanelActionButtons) mScreen.findViewById(R.id.action_buttons);
        if (mActionBtns == null) {
            throw new IllegalArgumentException("action buttons do not exist");
        }

        mFallBackPanel = (VTPanelFallBackPanel) mScreen.findViewById(R.id.vtpanel_fallback_panel);
        if (mFallBackPanel == null) {
            throw new IllegalArgumentException("fallback buttons do not exist");
        }

        return false;
    }

    void initSubComponents() {
        // arrows
        /*
         * mArrowLocal2Remote = (ImageView)
         * mScreen.findViewById(R.id.arrow_local_2_remote);
         * mArrowLocal2Remote.setVisibility(View.GONE); mArrowRemote2Local =
         * (ImageView) mScreen.findViewById(R.id.arrow_remote_2_local);
         * mArrowRemote2Local.setVisibility(View.GONE);
         */

        // middle buttons
        mNameView = (TextView) mScreen.findViewById(R.id.vt_name);
        mPhoneNumberView = (TextView) mScreen.findViewById(R.id.vt_phone_number);
        mUserInputView = (EditText) mScreen.findViewById(R.id.txt_view_userinput);
        mUserInputView.setVisibility(View.GONE);

        /*
         * if("qwertybar".equals(SystemProperties.get("hw.formfactor")) ||
         * mScreen.getResources().getConfiguration().hardKeyboardHidden==
         * mScreen.getResources().getConfiguration().HARDKEYBOARDHIDDEN_NO) {
         * mUserInputView.setVisibility(View.VISIBLE);
         * mUserInputView.setCursorVisible(false); } else {
         */
        mUserInputView.setCursorVisible(true);
        mUserInputView.setInputType(InputType.TYPE_NULL);
        // }
        mUserInputView.setKeyListener(PhoneKeyListener.getInstance());
        mUserInputView.addTextChangedListener(inputWatcher);

        // mUserKeyPadInputView.setKeyListener(PhoneKeyListener.getInstance());
        // mUserKeyPadInputView.addTextChangedListener(inputWatcher);

        mCardTitleView = (TextView) mScreen.findViewById(R.id.vt_card_title);
        if (mScreen.mDialOrAnswer) {
            // MO
            mCardTitleView.setText(R.string.mo_call);
        } else {
            // MT
            mCardTitleView.setText(R.string.mt_call);
        }

        mRepImage = (ImageView) mScreen.findViewById(R.id.surface_anim_photo);
        setCameraDlg = new SetCameraDialog(mScreen);
        setCameraDlg.setCancelable(false);

        mBtnEndCall = mScreen.findViewById(R.id.btnEndCall);
        Button btnEndCall = (Button) mScreen.findViewById(R.id.btn_end_call);
        btnEndCall.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                mScreen.onEndCall();
            }
        });

        mBtnCloseKeyboard = mScreen.findViewById(R.id.btnCloseKeyboard);
        ImageButton btnCloseKeyboard = (ImageButton) mScreen.findViewById(R.id.btn_close_keyboard);
        btnCloseKeyboard.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                /*
                 * if("qwertybar".equals(SystemProperties.get("hw.formfactor"))||
                 * mScreen
                 * .getResources().getConfiguration().hardKeyboardHidden==
                 * mScreen
                 * .getResources().getConfiguration().HARDKEYBOARDHIDDEN_NO) {
                 * mUserInputView.setVisibility(View.VISIBLE); } else {
                 * mUserInputView.setVisibility(View.GONE); }
                 */
        mScreen.onCloseKeyboard();
        mUserInputView.setVisibility(View.GONE);
        mCtrlBtns.setVisibility(View.VISIBLE);
        mActionBtns.setVisibility(View.VISIBLE);
            }
        });
        mBtnCallingEndCall = mScreen.findViewById(R.id.btnCallingEndcall);
        Button btnCallingEndCall = (Button) mScreen.findViewById(R.id.btn_calling_end_call);
        btnCallingEndCall.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                mScreen.onEndCall();
            }
        });
        mBtnOpenKeyboard = mScreen.findViewById(R.id.btnOpenKeyboard);
        ImageButton btnOpenKeyboard = (ImageButton) mScreen.findViewById(R.id.btn_open_keyboard);
        btnOpenKeyboard.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
        mScreen.onShowKeyboard();
        mUserInputView.setVisibility(View.VISIBLE);
        mCtrlBtns.setVisibility(View.GONE);
        mActionBtns.setVisibility(View.GONE);
            }
        });
        if ("qwertybar".equals(SystemProperties.get("hw.formfactor"))) {
            if (btnEndCall != null) {
                btnEndCall.setFocusable(false);
            }
            if (btnCloseKeyboard != null) {
                btnCloseKeyboard.setFocusable(false);
            }
            if (btnCallingEndCall != null) {
                btnCallingEndCall.setFocusable(false);
            }
            if (btnOpenKeyboard != null) {
                btnOpenKeyboard.setFocusable(false);
            }

            ImageButton btnTouchOne = (ImageButton) mScreen.findViewById(R.id.touch_one);
            if (btnTouchOne != null) {
                btnTouchOne.setFocusable(false);
            }
            ImageButton btnTouchTwo = (ImageButton) mScreen.findViewById(R.id.touch_two);
            if (btnTouchTwo != null) {
                btnTouchTwo.setFocusable(false);
            }
            ImageButton btnTouchThree = (ImageButton) mScreen.findViewById(R.id.touch_three);
            if (btnTouchThree != null) {
                btnTouchThree.setFocusable(false);
            }
            ImageButton btnTouchFour = (ImageButton) mScreen.findViewById(R.id.touch_four);
            if (btnTouchFour != null) {
                btnTouchFour.setFocusable(false);
            }
            ImageButton btnTouchFive = (ImageButton) mScreen.findViewById(R.id.touch_five);
            if (btnTouchFive != null) {
                btnTouchFive.setFocusable(false);
            }
            ImageButton btnTouchSix = (ImageButton) mScreen.findViewById(R.id.touch_six);
            if (btnTouchSix != null) {
                btnTouchSix.setFocusable(false);
            }
            ImageButton btnTouchSeven = (ImageButton) mScreen.findViewById(R.id.touch_seven);
            if (btnTouchSeven != null) {
                btnTouchSeven.setFocusable(false);
            }
            ImageButton btnTouchEight = (ImageButton) mScreen.findViewById(R.id.touch_eight);
            if (btnTouchEight != null) {
                btnTouchEight.setFocusable(false);
            }
            ImageButton btnTouchNine = (ImageButton) mScreen.findViewById(R.id.touch_nine);
            if (btnTouchNine != null) {
                btnTouchNine.setFocusable(false);
            }
            ImageButton btnTouchStar = (ImageButton) mScreen.findViewById(R.id.touch_star);
            if (btnTouchStar != null) {
                btnTouchStar.setFocusable(false);
            }
            ImageButton btnTouchZero = (ImageButton) mScreen.findViewById(R.id.touch_zero);
            if (btnTouchZero != null) {
                btnTouchZero.setFocusable(false);
            }
            ImageButton btnTouchPound = (ImageButton) mScreen.findViewById(R.id.touch_pound);
            if (btnTouchPound != null) {
                btnTouchPound.setFocusable(false);
            }
        }
        setMiddleButtonsVisibility();

        mBtnCapturePhoto = mScreen.findViewById(R.id.btn_capture_photo);
        mBtnCapturePhoto.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View view, MotionEvent event) {
                // TODO Auto-generated method stub
                if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
                {
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
                    break;
                default:
                    break;
                    }
                }
                else{
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mBtnCapturePhoto.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    mBtnCapturePhoto.setBackgroundResource(0);
                    break;
                case MotionEvent.ACTION_UP:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "Begin CapturePhoto");
                    mScreen.onCapturePhoto();
                    mMainPanel.cancelDelayedSwitchMode();
                    mBtnCapturePhoto.setBackgroundResource(0);
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "End CapturePhoto");
                    // mMainPanel.delaySwitchMode(VideoCallApp.sPendingPanelMode,
                    // VTPanel.DELAY_SWITCH_MODE_SHORT_TIME);
                    break;
                default:
                    break;
                        }
                    }
                return true;
        }
        });

		cameraImg = (ImageView)mScreen.findViewById(R.id.camera_control);
		cameraImg.setOnTouchListener(new View.OnTouchListener() {
		    public boolean onTouch(View view, MotionEvent event) {
			    if (mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE) {
				if (MyLog.DEBUG)
				    MyLog.d(TAG, "App in idle status, just return");
					    return true;
				}
				if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
				{
				switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
				    break;
				case MotionEvent.ACTION_CANCEL:
					break;
				case MotionEvent.ACTION_UP:
					break;
				default:
					break;
					}
				}
				else{
				switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
				cameraImg.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
					break;
				case MotionEvent.ACTION_CANCEL:
					cameraImg.setBackgroundResource(0);
					break;
				case MotionEvent.ACTION_UP:
					if (!isProgressing) {
                    if (MyLog.DEBUG)
                    MyLog.v(TAG, "mBtnOpenCloseCamera event: "
                            + VideoCallApp.sPendingPanelMode);
                    isProgressing = true;
                    progressDlg = ProgressDialog.show(mScreen, "",
                            mScreen.getText(R.string.please_wait_msg), true, false);
					if (mApp.mCurrentVideoSource != VTManager.VideoSource.CAMERA_NONE)
					{
			            cameraImg.setImageResource(R.drawable.camera_off);
		            }
					else
					{
					    cameraImg.setImageResource(R.drawable.camera_on);
					}
                    Thread task = new Thread(new OpenCloseCameraTask());
                    task.start();
					}
					break;
				default:
					break;
						}
					}
					return true;
					}
				});


        mBtnOpenCloseCamera = mScreen.findViewById(R.id.btn_close_camera);
        mBtnOpenCloseCamera.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View view, MotionEvent event) {
                if (mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE) {
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "App in idle status, just return");
                    return true;
                }
                if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
                {
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
                    break;
                default:
                    break;
                    }
                }
                else{
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mBtnOpenCloseCamera
                            .setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    mBtnOpenCloseCamera.setBackgroundResource(0);
                    break;
                case MotionEvent.ACTION_UP:
                    if (!isProgressing) {
                        if (MyLog.DEBUG)
                            MyLog.v(TAG, "mBtnOpenCloseCamera event: "
                                    + VideoCallApp.sPendingPanelMode);
                        // hideSurfaceControlButtons();
                        isProgressing = true;
                        focusButton = mBtnOpenCloseCamera;
                        progressDlg = ProgressDialog.show(mScreen, "",
                                mScreen.getText(R.string.please_wait_msg), true, false);
                        Thread task = new Thread(new OpenCloseCameraTask());
                        task.start();
                    }
                    break;
                default:
                    break;
                }
            }
                return true;
            }
        });

        mBtnSetCamera = mScreen.findViewById(R.id.btn_set_camera);
        mBtnSetCamera.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View view, MotionEvent event) {
                // TODO Auto-generated method stub

                if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
                {
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
                    break;
                default:
                    break;
                }
                }
                else{
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mBtnSetCamera
                            .setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    mBtnSetCamera.setBackgroundResource(0);
                    break;
                case MotionEvent.ACTION_UP:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "SetCameraDlg begin show");
                    mBtnSetCamera.setBackgroundResource(0);
                    //mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);
                    if (setCameraDlg != null) {
                        setCameraDlg.dismiss();
                        setCameraDlg = null;
                        }
                        setCameraDlg = new SetCameraDialog(mScreen);
                        setCameraDlg.setCancelable(false);
                        setCameraDlg.show();
                    break;
                default:
                    break;
                }
                }
                return true;
            }
        });

        mLocalWin.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View v, MotionEvent event) {
            if ((VideoCallApp.sPanelMode+2 == VTPanel.VTPANEL_MODE_REMOTE)||(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE)){
                // TODO Auto-generated method stub
               switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
                    onSwapWindow();
                    break;
                default:
                    break;
                }
                }
                return true;
            }
        });

        mRemoteWin.setOnTouchListener(new View.OnTouchListener() {
        public boolean onTouch(View v, MotionEvent event) {
        if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_LOCAL){
                // TODO Auto-generated method stub
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
                    onSwapWindow();
                    break;
                default:
                    break;
                }
                }
                return true;
            }
        });

        mBtnSwitchSpeaker = mScreen.findViewById(R.id.btn_switch_speaker);
        // TextView text = (TextView)
        // mScreen.findViewById(R.id.txt_switch_speaker);
        toSpeaker = (ImageView) mScreen.findViewById(R.id.img_switch_speaker);
        txtSpeaker = (TextView) mScreen.findViewById(R.id.textSpeaker);
        // text.setText(R.string.switch_to_speaker);
        // tospeaker.setImageResource(R.drawable.cmcc_toolbar_microphone);
        updateSpeakerStatus();
        mBtnSwitchSpeaker.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View view, MotionEvent event) {
                AudioManager mAudioManager = (AudioManager) mApp
                        .getSystemService(Context.AUDIO_SERVICE);

        if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
        {
        switch (event.getAction()) {
        case MotionEvent.ACTION_DOWN:
            break;
        case MotionEvent.ACTION_CANCEL:
            break;
        case MotionEvent.ACTION_UP:
            break;
        default:
            break;
        }
        }
        else{
        switch (event.getAction()) {
        case MotionEvent.ACTION_DOWN:
            mBtnSwitchSpeaker.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                break;
        case MotionEvent.ACTION_CANCEL:
            mBtnSwitchSpeaker.setBackgroundResource(0);
                break;
        case MotionEvent.ACTION_UP:
            mBtnSwitchSpeaker.setBackgroundResource(0);
            if (VTServiceInterface.isSpeakerOn(mApp.getApplicationContext())) {
            if (mApp.isBtHeadsetPlugged() == true) {
                        mApp.switchBtSco(true);
                    } else if (mAudioManager.isWiredHeadsetOn()) {
                        Log.e(TAG, "switchSpeaker: switch wired headset on");
                        final VideoCallApp mApp = VideoCallApp.getInstance();
                        final Handler mAppHandler = VideoCallApp.getInstance().mHandler;
                        mAppHandler.sendMessage(mAppHandler.obtainMessage(
                                VideoCallApp.MSG_WIRED_HEADSET_PLUG, 1, 0));

                    } else {
                        if (MyLog.DEBUG)
                            Log.e(TAG, "switchSpeaker: Switching off the speker");
                        mApp.switchSpeaker(false);
                    }
                }else{
                    if (mApp.isBtHeadsetPlugged() == true) {
                        mApp.switchBtSco(false);
                    }
                    Log.e(TAG, "switchSpeaker: Switching on the speker");
                    mApp.switchSpeaker(true);
                    if (mAudioManager.isWiredHeadsetOn()) {
                        final VideoCallApp mApp = VideoCallApp.getInstance();
                        final Handler mAppHandler = VideoCallApp.getInstance().mHandler;
                        mAppHandler.sendMessage(mAppHandler.obtainMessage(
                                VideoCallApp.MSG_WIRED_HEADSET_PLUG, 0, 0));
                    }
                }
                    break;
                default:
                    break;
                }
                }
                return true;
            }
        });

        // mBtnRemoteOnly = mScreen.findViewById(R.id.btn_remote_only);
        // mBtnRemoteOnly.setOnTouchListener(new View.OnTouchListener() {
        // public boolean onTouch(View v, MotionEvent event) {
        // // TODO Auto-generated method stub
        // switch(event.getAction()){
        // case MotionEvent.ACTION_DOWN:
        // mBtnRemoteOnly.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_pressed);
        // break;
        // case MotionEvent.ACTION_CANCEL:
        // mBtnRemoteOnly.setBackgroundResource(0);
        // break;
        // case MotionEvent.ACTION_UP:
        // if (!isProgressing) {
        // isProgressing = true;
        // isRemoteOnly = true;
        // focusButton = mBtnRemoteOnly;
        // progressDlg =
        // ProgressDialog.show(mScreen,"",mScreen.getText(R.string.please_wait_msg),
        // true, false);
        // Thread task = new Thread(new CloseLocalVideoTask());
        // task.start();
        // }
        // break;
        // }
        // return true;
        // }
        // });

        // fall back panel

        // mChkFallBack.setChecked(false);
    }

    void switchPanelMode(int newMode, int milliseconds) {
        MyLog.d(TAG, "switchPanelMode: " + newMode);
        // hideSurfaceControlButtons();
        mMainPanel.switchMode(newMode, milliseconds);
        // change background
        setMiddleButtonsVisibility();
    }

    void switchPanelMode(int newMode) {
        this.switchPanelMode(newMode, VTPanel.ANIMATION_DURATION);
    }

    void cancelSwitchPanel() {
        if (mMainPanel != null) {
            mMainPanel.cancelDelayedSwitchMode();
        }
    }

    public boolean onFlingAction(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        // TODO Auto-generated method stub
        MyLog.d(TAG, "components on fling...e1: " + e1 + " e2: " + e2);
        return false;
    }

    private void log(String info) {
        if (DBG) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, info);
        }
    }

    public void onSwapWindow() {

        // TODO Auto-generated method stub
        /*
         * if (mLocalWin.equals(target) && VideoCallApp.sPanelMode ==
         * VTPanel.VTPANEL_MODE_LOCAL) { // show arrow local->remote
         * switchPanelMode(VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_LOCAL); }
         * else if (mRemoteWin.equals(target) && VideoCallApp.sPanelMode ==
         * VTPanel.VTPANEL_MODE_REMOTE) { // show arrow remote->local
         * switchPanelMode(VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_REMOTE); }
         * else
         */

        // if (mRemoteWin.equals(target) && VideoCallApp.sPanelMode ==
        // VTPanel.VTPANEL_MODE_REMOTE_ONLY) {
        // switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
        // // Work-around for new CameraHal, we have to set local surface
        // visible when VideoCallScreen.onStop.
        // if (mApp.getVTServiceInterface().isUseVTCamera()) {
        // mScreen.setLocalSurfaceVisibility(View.GONE);
        // }
        // //
        // mScreen.setLocalSurfaceVisibility(View.VISIBLE);
        // }
        if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE) {
            switchPanelMode(VTPanel.VTPANEL_MODE_LOCAL);
            mRemoteWin.bringToFront();
        } else if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_LOCAL) {
            // show arrow remote->local
            switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
            mLocalWin.bringToFront();
        }
    }

    public void onClickEvent(VTPanelComponent target) {
        log("onclickevent...");
        // TODO Auto-generated method stub
        /*
         * if (mLocalWin.equals(target) && VideoCallApp.sPanelMode ==
         * VTPanel.VTPANEL_MODE_LOCAL) { // show arrow local->remote
         * switchPanelMode(VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_LOCAL); }
         * else if (mRemoteWin.equals(target) && VideoCallApp.sPanelMode ==
         * VTPanel.VTPANEL_MODE_REMOTE) { // show arrow remote->local
         * switchPanelMode(VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_REMOTE); }
         * else
         */if (mRemoteWin.equals(target)
                && VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE_ONLY) {
            switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
            // Work-around for new CameraHal, we have to set local surface
            // visible when VideoCallScreen.onStop.
            if (mApp.getVTServiceInterface().isUseVTCamera()) {
                mScreen.setLocalSurfaceVisibility(View.GONE);
            }
            //
            mScreen.setLocalSurfaceVisibility(View.VISIBLE);
        }

        // if (mLocalWin.equals(target) && VideoCallApp.sPanelMode ==
        // VTPanel.VTPANEL_MODE_REMOTE) {
        // switchPanelMode(VTPanel.VTPANEL_MODE_LOCAL);
        // } else if (mRemoteWin.equals(target) && VideoCallApp.sPanelMode ==
        // VTPanel.VTPANEL_MODE_LOCAL) {
        // // show arrow remote->local
        // switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
        // }

    }

    public void onTouchDownEvent(VTPanelComponent target, MotionEvent ev) {
        log("onTouchDownEvent...");
        // TODO Auto-generated method stub
    }

    public void onTouchCancelEvent(VTPanelComponent target, MotionEvent ev) {
        // TODO Auto-generated method stub
        /*
         * if ((mLocalWin.equals(target) && VideoCallApp.sPanelMode ==
         * VTPanel.VTPANEL_MODE_REMOTE) || (mRemoteWin.equals(target) &&
         * VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_LOCAL)) { // show
         * arrow local->remote mArrowLocal2Remote.setVisibility(View.GONE);
         * mArrowRemote2Local.setVisibility(View.GONE); }
         */
    }

    public void onAnimationStart() {
        // TODO Auto-generated method stub
        int mode = VideoCallApp.sPanelMode;
        mCtrlBtns.mVisibilities[mode] = View.GONE;
        mScreen.onVTPanelNotify(VTPANEL_EVENT_ANIMATION_START);
    }

    public void onAnimationEnd() {
        // TODO Auto-generated method stub
        log("onAnimationend...mode: " + VideoCallApp.sPanelMode + " pendingmode: "
                + VideoCallApp.sPendingPanelMode);
        int orientation = mScreen.getWindow().getWindowManager().getDefaultDisplay()
                .getOrientation();
        mCardTitleView.setVisibility(View.VISIBLE);
        mCardTitleView.setGravity(Gravity.RIGHT);
        mNameView.setVisibility(View.VISIBLE);
        switch (VideoCallApp.sPanelMode) {

        /*
         * case VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_LOCAL:
         * mCtrlBtns.mVisibilities[VideoCallApp.sPanelMode] = View.VISIBLE;
         *
         * //mBtnRecordVideo.setVisibility(View.GONE);
         * mBtnCapturePhoto.setVisibility(View.GONE);
         * //mBtnRecordAudio.setVisibility(View.GONE);
         * mBtnSwitchSpeaker.setVisibility(View.GONE);
         * mBtnRemoteOnly.setVisibility(View.GONE);
         * mBtnOpenCloseCamera.setVisibility(View.VISIBLE);
         * //mBtnSwitchCamera.setVisibility(View.VISIBLE);
         * mBtnSwitchCamera.setVisibility(View.GONE);
         * mBtnSetCamera.setVisibility(View.VISIBLE); break; case
         * VTPanel.VTPANEL_MODE_CONTROL_BUTTON_RIGHT_REMOTE:
         * mCtrlBtns.mVisibilities[VideoCallApp.sPanelMode] = View.VISIBLE;
         *
         * //mBtnRecordVideo.setVisibility(View.GONE);
         * mBtnCapturePhoto.setVisibility(View.VISIBLE);
         * //mBtnRecordAudio.setVisibility(View.GONE);
         * mBtnSwitchSpeaker.setVisibility(View.VISIBLE);
         * mBtnRemoteOnly.setVisibility(View.VISIBLE);
         * mBtnOpenCloseCamera.setVisibility(View.GONE);
         * mBtnSwitchCamera.setVisibility(View.GONE);
         * mBtnSetCamera.setVisibility(View.GONE); break;
         */
        case VTPanel.VTPANEL_MODE_LOCAL:
            checkIfShowInputView(orientation);
            // mBtnRecordVideo.setVisibility(View.GONE);
            mBtnCapturePhoto.setVisibility(View.VISIBLE);
            // mBtnRemoteWindow.setVisibility(View.GONE);
            // mBtnRecordAudio.setVisibility(View.GONE);
            // mBtnSwitchSpeaker.setVisibility(View.GONE);
            // mBtnRemoteOnly.setVisibility(View.GONE);
            mBtnOpenCloseCamera.setVisibility(View.VISIBLE);
            // mBtnSwitchCamera.setVisibility(View.GONE);
            // mBtnSwitchCamera.setVisibility(View.VISIBLE);
            mBtnSetCamera.setVisibility(View.VISIBLE);
            // mImgSwapWindow.setVisibility(View.VISIBLE);

            if (VideoCallApp.sPendingPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_LOCAL) {
                mScreen.onVTPanelNotify(VTPANEL_EVENT_SHOW_ACTION_BUTTON);
            }
            break;
        case VTPanel.VTPANEL_MODE_REMOTE:
            checkIfShowInputView(orientation);
            // mBtnRecordVideo.setVisibility(View.GONE);
            mBtnCapturePhoto.setVisibility(View.VISIBLE);
            // mBtnRemoteWindow.setVisibility(View.VISIBLE);
            // mBtnRecordAudio.setVisibility(View.GONE);
            // mBtnSwitchSpeaker.setVisibility(View.VISIBLE);
            // mBtnRemoteOnly.setVisibility(View.VISIBLE);
            mBtnOpenCloseCamera.setVisibility(View.VISIBLE);
            // mBtnSwitchCamera.setVisibility(View.GONE);
            mBtnSetCamera.setVisibility(View.VISIBLE);
            // mImgSwapWindow.setVisibility(View.VISIBLE);
            if (VideoCallApp.sPendingPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE) {
                mScreen.onVTPanelNotify(VTPANEL_EVENT_SHOW_ACTION_BUTTON);
            }
            break;
        case VTPanel.VTPANEL_MODE_REMOTE_ONLY:
            mCardTitleView.setVisibility(View.VISIBLE);
            mCardTitleView.setGravity(Gravity.RIGHT);
            // mNameView.setVisibility(View.INVISIBLE);
            // mPhoneNumberView.setVisibility(View.INVISIBLE);
            checkIfShowInputView(orientation);
            if (VideoCallApp.sPendingPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY) {
                mScreen.onVTPanelNotify(VTPANEL_EVENT_SHOW_ACTION_BUTTON);
            }
            break;

        case VTPanel.VTPANEL_MODE_KEYPAD_LOCAL:
        case VTPanel.VTPANEL_MODE_KEYPAD_REMOTE:
            mScreen.onVTPanelNotify(VTPANEL_EVENT_SHOW_KEYPAD);
            // mUserInputView.setVisibility(View.VISIBLE);
            mUserInputView.requestFocus();
            // mImgSwapWindow.setVisibility(View.VISIBLE);
            break;
        case VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY:
            mScreen.onVTPanelNotify(VTPANEL_EVENT_SHOW_KEYPAD);
            // mUserInputView.setVisibility(View.VISIBLE);
            mUserInputView.requestFocus();
            mCardTitleView.setVisibility(View.INVISIBLE);
            // mNameView.setVisibility(View.INVISIBLE);
            break;
        default:
            break;
        }
        mScreen.onVTPanelNotify(VTPANEL_EVENT_ANIMATION_DONE);
    }

    void disableControlCamera() {
        // mBtnSwitchCamera.setEnabled(false);
        mBtnSetCamera.setEnabled(false);
        // mBtnLocalWindow.setEnabled(false);
    }

    void enableControlCamera() {
        // mBtnSwitchCamera.setEnabled(true);
        mBtnSetCamera.setEnabled(true);
        // mBtnLocalWindow.setEnabled(true);
    }

    void showStartRecordAudioState() {
        // mRecordImage.setVisibility(View.VISIBLE);
    }

    void showEndRecordAudioState() {
        // mRecordImage.setVisibility(View.GONE);
    }

    // FIXME: check below logics
    /**
     * update call card ui using call infor searched name and number can be null
     */
    void updateCallInfoUI(String name, String number) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "updateCallInfoUI...name: " + name + ", number: " + number);
        if (name != null) {
            if (mNameView != null && mPhoneNumberView != null) {
                mNameView.setText(name);
                mPhoneNumberView.setText(number);
            }
        } else {
            if (mNameView != null && mPhoneNumberView != null) {
                mNameView.setText(number);
                mPhoneNumberView.setText("");
            }
        }
    }

    void setUserInput(String data) {
        mUserInputView.setText("");
        mUserInputView.append(data);

        /*
         * mUserKeyPadInputView.setText(""); mUserKeyPadInputView.append(data);
         */
    }

    void appendUserInput(String data) {
        mUserInputView.append(data);
        // mUserKeyPadInputView.append(data);
    }

    void setCallCardTitle(String data) {
        mCardTitleView.setText(data);
    }

    private void setMiddleButtonsVisibility() {
        switch (VideoCallApp.sPanelMode) {
        case VTPanel.VTPANEL_MODE_LOCAL:
        case VTPanel.VTPANEL_MODE_REMOTE:
        case VTPanel.VTPANEL_MODE_REMOTE_ONLY:
            mMainPanel.setBackgroundResource(R.drawable.cmcc_phone_shadow);
            if (mBtnCallingEndCall != null)
                mBtnCallingEndCall.setVisibility(View.VISIBLE);
            if (mBtnOpenKeyboard != null)
                mBtnOpenKeyboard.setVisibility(View.VISIBLE);
            if (mBtnCloseKeyboard != null)
                mBtnCloseKeyboard.setVisibility(View.GONE);
            if (mBtnEndCall != null)
                mBtnEndCall.setVisibility(View.GONE);
            break;
        case VTPanel.VTPANEL_MODE_KEYPAD_LOCAL:
        case VTPanel.VTPANEL_MODE_KEYPAD_REMOTE:
        case VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY:
            mMainPanel.setBackgroundResource(0);
            if (mBtnCallingEndCall != null)
                mBtnCallingEndCall.setVisibility(View.GONE);
            if (mBtnOpenKeyboard != null)
                mBtnOpenKeyboard.setVisibility(View.GONE);
            if (mBtnCloseKeyboard != null)
                mBtnCloseKeyboard.setVisibility(View.VISIBLE);
            if (mBtnEndCall != null)
                mBtnEndCall.setVisibility(View.VISIBLE);
            break;
        case VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING:
            if (mScreen.mDialOrAnswer) {
                // call
                if (mBtnCallingEndCall != null)
                    mBtnCallingEndCall.setVisibility(View.VISIBLE);
                if (mBtnOpenKeyboard != null)
                    mBtnOpenKeyboard.setVisibility(View.VISIBLE);
                if (mBtnCloseKeyboard != null)
                    mBtnCloseKeyboard.setVisibility(View.VISIBLE);
                if (mBtnEndCall != null)
                    mBtnEndCall.setVisibility(View.VISIBLE);
            } else {
                // answer
                if (mBtnCallingEndCall != null)
                    mBtnCallingEndCall.setVisibility(View.GONE);
                if (mBtnOpenKeyboard != null)
                    mBtnOpenKeyboard.setVisibility(View.GONE);
                if (mBtnCloseKeyboard != null)
                    mBtnCloseKeyboard.setVisibility(View.GONE);
                if (mBtnEndCall != null)
                    mBtnEndCall.setVisibility(View.GONE);
            }
            break;
        case VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING:
            break;
        default:
            break;
        }
    }

    void onSwitchCamera() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "on switch camera");

        if (mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "App in idle status, just return");
            return;
        }
        if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_MAIN) {
            try {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "switch from main camera");
                mApp.mVTService.setVideoSource(VTManager.VideoSource.CAMERA_SECONDARY, null, null);
                mApp.mLastVideoSource = mApp.mCurrentVideoSource;
                mApp.mCurrentVideoSource = VTManager.VideoSource.CAMERA_SECONDARY;
                mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);

                mApp.mCameraSetting.updateCameraParams(mApp.mVTService,
                        CameraSetting.CONSTRAST_SETTING,
                        mApp.mCameraSetting.getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
                mApp.mCameraSetting.updateCameraParams(mApp.mVTService,
                        CameraSetting.BRIGHT_SETTING,
                        mApp.mCameraSetting.getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
                // mSkBarContast.setMax( mApp.mCameraSetting.getSettingDegree(
                // CameraSetting.CONSTRAST_SETTING));
                // mSkBarBright.setMax( mApp.mCameraSetting.getSettingDegree(
                // CameraSetting.BRIGHT_SETTING));
                // mVTService.sendUserInput(
                // mApp.MSG_SWITCH_TO_SECONDARY_CAMERA);
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "switch to secondary camera");
            } catch (Exception e) {
                // TODO: handle exception
                Log.e(TAG, e.toString());
                return;
            }
        } else if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_SECONDARY) {
            try {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "switch from secondary");
                mApp.mVTService.setVideoSource(VTManager.VideoSource.CAMERA_MAIN, null, null);
                mApp.mLastVideoSource = mApp.mCurrentVideoSource;
                mApp.mCurrentVideoSource = VTManager.VideoSource.CAMERA_MAIN;
                mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);

                mApp.mCameraSetting.updateCameraParams(mApp.mVTService,
                        CameraSetting.CONSTRAST_SETTING,
                        mApp.mCameraSetting.getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
                mApp.mCameraSetting.updateCameraParams(mApp.mVTService,
                        CameraSetting.BRIGHT_SETTING,
                        mApp.mCameraSetting.getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
                mApp.mCameraSetting.updateCameraParams(mApp.mVTService,
                        CameraSetting.CONSTRAST_SETTING,
                        mApp.mCameraSetting.getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
                // mSkBarContast.setMax( mApp.mCameraSetting.getSettingDegree(
                // CameraSetting.CONSTRAST_SETTING));
                // mSkBarBright.setMax( mApp.mCameraSetting.getSettingDegree(
                // CameraSetting.BRIGHT_SETTING));
                // mVTService.sendUserInput( mApp.MSG_SWITCH_TO_MAIN_CAMERA);
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "switch to main camera");
            } catch (Exception e) {
                // TODO: handle exception
                Log.e(TAG, e.toString());
            }
        }
    }

    public void updateSpeakerStatus() {
        if (VTServiceInterface.isSpeakerOn(mApp.getApplicationContext())) {
            toSpeaker.setImageResource(R.drawable.ic_speaker_presses);
            txtSpeaker.setText(R.string.switch_to_speaker);
        } else {
            toSpeaker.setImageResource(R.drawable.ic_speaker);
            txtSpeaker.setText(R.string.switch_to_speaker_off);
        }
    }


    void closeRepImage() {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "closeRepImage");
        mRepImage.setVisibility(View.GONE);
    }

    void showRepImage() {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "showRepImage");
        Bitmap bm = null;
        try {
            int imagesize = 176 * 144 * 4;
            byte[] buffer = new byte[imagesize];
            FileInputStream input;
            File f = mApp.getApplicationContext().getFileStreamPath(VTImageReplaceSetting.mStrFN);
            if (!f.exists()) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "VTImg.row does not exist, create new");
                VTImageReplaceSetting.setDefaultImage(mApp.getApplicationContext());
            }
            input = mScreen.openFileInput(VTImageReplaceSetting.mStrFN);

            int ns = input.read(buffer);
            if (MyLog.DEBUG)
                MyLog.v(TAG, "Read bytes " + ns);
            if (ns != imagesize) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "exit checkItemImage because the wrong file size.");
                return;
            }

            // to update the image content
            ByteBuffer btBuf = ByteBuffer.wrap(buffer);

            bm = Bitmap.createBitmap(176, 144, Bitmap.Config.ARGB_8888);
            for (int y = 0; y < 144; y++)
                for (int x = 0; x < 176; x++)
                    bm.setPixel(x, y, btBuf.getInt());
            input.close();
        } catch (FileNotFoundException e) {
            Log.e(TAG, "File not found, " + e.getMessage());
        } catch (IOException eio) {
            Log.e(TAG, "Failed to read, " + eio.getMessage());
        }

        if (bm != null) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "setImageBitmap: " + bm);
            mRepImage.setScaleType(ImageView.ScaleType.FIT_XY);
            mRepImage.setImageBitmap(bm);
            mRepImage.setVisibility(View.VISIBLE);
        }
    }

    void checkIfShowInputView(int orientation) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "please tell me the orientation" + orientation);
        if (orientation == 0 || orientation == 2) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "portrait mode");
            /*
             * if("qwertybar".equals(SystemProperties.get("hw.formfactor"))||
             * mScreen.getResources().getConfiguration().hardKeyboardHidden==
             * mScreen.getResources().getConfiguration().HARDKEYBOARDHIDDEN_NO)
             * { mUserInputView.setVisibility(View.VISIBLE); } else {
             * mUserInputView.setVisibility(View.GONE); }
             */
        } else {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "landscape mode");
            // mUserInputView.setVisibility(View.VISIBLE);
            mUserInputView.requestFocus();
        }
    }
}
