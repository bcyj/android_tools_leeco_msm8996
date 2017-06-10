/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.util.HashMap;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;

public class VideoCallKeyPad extends LinearLayout {

	static final String TAG = "VT/VideoCallKeyPad";

	protected OnClickListener mClickListener = new ButtonClickListener();
	protected OnLongClickListener mLongClickListener = new ButtonLongClickListener();

	protected KeypadClickListener mKeypadClickListener;
	protected KeypadLongClickListener mKeypadLongClickListener;

	/** Hash Map to map a view id to a character */
	private static final HashMap<Integer, Character> mDisplayMap = new HashMap<Integer, Character>();

	{
		// Map the buttons to the display characters
		mDisplayMap.put(R.id.touch_one, '1');
		mDisplayMap.put(R.id.touch_two, '2');
		mDisplayMap.put(R.id.touch_three, '3');
		mDisplayMap.put(R.id.touch_four, '4');
		mDisplayMap.put(R.id.touch_five, '5');
		mDisplayMap.put(R.id.touch_six, '6');
		mDisplayMap.put(R.id.touch_seven, '7');
		mDisplayMap.put(R.id.touch_eight, '8');
		mDisplayMap.put(R.id.touch_nine, '9');
		mDisplayMap.put(R.id.touch_zero, '0');
		mDisplayMap.put(R.id.touch_pound, '#');
		mDisplayMap.put(R.id.touch_star, '*');
	}

	public VideoCallKeyPad(Context context, AttributeSet attrs) {
		super(context, attrs);
		inflateKeypad(this);
	}

	/**
	 * inflate the view hierarchy for this object according to the resource xml
	 * file
	 * <p>
	 * Note:in general, you do not need to call this function directly, add this
	 * class to your layout file, it will be inflated automatically
	 *
	 * @param parent
	 *            the container which will include the inflated view
	 * @return the TwelveKeypad view which has been fully inflated
	 */
	protected View inflateKeypad(VideoCallKeyPad parent) {
		if (MyLog.DEBUG) MyLog.d(TAG, "inflateKeypad..");
		if (parent == null)
			throw new AssertionError("argument could not be null");

		LayoutInflater viewInflater = LayoutInflater.from(parent.getContext());
		// if (MyLog.DEBUG) MyLog.d(TAG,"retrieve the ViewInflate object " + viewInflater );

		if (viewInflater == null) {
			throw new AssertionError("fail to retrieve ViewInflate object");
		}

		VideoCallKeyPad keypad = null;

		keypad = (VideoCallKeyPad) viewInflater.inflate(
				R.layout.video_call_keypad, parent, true);

		keypad.init();

		return keypad;
	}

	/**
	 * initialize the click listener for each button
	 */
	private void init() {
		View view = findViewById(R.id.touch_one);
		view.setOnClickListener(mClickListener);
		view.setTag(Character.valueOf('1'));
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_two);
		view.setOnClickListener(mClickListener);
		view.setTag(Character.valueOf('2'));
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_three);
		view.setTag(Character.valueOf('3'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_four);
		view.setTag(Character.valueOf('4'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_five);
		view.setTag(Character.valueOf('5'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_six);
		view.setTag(Character.valueOf('6'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_seven);
		view.setTag(Character.valueOf('7'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_eight);
		view.setTag(Character.valueOf('8'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_nine);
		view.setTag(Character.valueOf('9'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_zero);
		view.setTag(Character.valueOf('0'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_star);
		view.setTag(Character.valueOf('*'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		view = findViewById(R.id.touch_pound);
		view.setTag(Character.valueOf('#'));
		view.setOnClickListener(mClickListener);
		view.setOnLongClickListener(mLongClickListener);

		// LayoutParams params =
		// (LinearLayout.LayoutParams)view.getLayoutParams();
		// if (MyLog.DEBUG) MyLog.d(TAG,"the layout params for pound in TwelveKeypad " +
		// params.width + "/" + params.height);

	}

	protected class ButtonClickListener implements OnClickListener {
		public void onClick(View v) {
			if (v == null) {
				return;
			}
			char c = getClickText(v);
			/* notify the client */
			if (mKeypadClickListener != null) {
				if (MyLog.DEBUG) MyLog.d(TAG, "notify listener with char:" + c);
				mKeypadClickListener.onClick(c);
			}
		}
	};

	protected class ButtonLongClickListener implements OnLongClickListener {

		public boolean onLongClick(View v) {
			if (v == null) {
				return false;
			}

			char c = getClickText(v);
			if (mKeypadLongClickListener != null) {
				if (MyLog.DEBUG) MyLog.d(TAG, "notify long-click listener with char:" + c);
				return mKeypadLongClickListener.onLongClick(c);
			}
			return false;
		}

	}

	private char getClickText(View v) {
		ImageButton but = (ImageButton) v;
		// String val = but.getText().toString();
		// depending on the text is a fragile design, because button's text may
		// never set.
		char c = ((Character) but.getTag()).charValue();
		if (MyLog.DEBUG) MyLog.d(TAG, "cilck " + but.getTag() + " return:" + c);

		return c;
	}

	public interface KeypadClickListener {
		/**
		 * called when one of the button in this soft keypad has been clicked
		 *
		 * @param c
		 *            the text of the button who has been clicked,currently we
		 *            just return the number text,not including letter
		 */
		void onClick(char c);

	}

	public interface KeypadLongClickListener {
		/**
		 * called when one of the button in this soft keypad has been long
		 * clicked
		 *
		 * @param c
		 *            the text of the button who has been long clicked,currently
		 *            we just return the number text,not including letter
		 */
		boolean onLongClick(char c);
	}

	public void setKeypadClickListener(KeypadClickListener listener) {
		mKeypadClickListener = listener;
	}

	public void setKeypadLongClickListener(KeypadLongClickListener listener) {
		mKeypadLongClickListener = listener;
	}

	public void setupTouchListener(View.OnTouchListener listener) {
		View imagebutton;
		for (int viewId : mDisplayMap.keySet()) {
			// locate the view
			imagebutton = findViewById(viewId);
			// Setup the listeners for the buttons
			imagebutton.setOnTouchListener(listener);
			imagebutton.setClickable(true);
		}
	}

	/**
	 * subclasses can use their own OnClickListener implementations to deal with
	 * the 'onClick' by default, it is a 'ButtonClickListener' instance
	 */
	protected void setNumButtonClickListener(OnClickListener listener) {
		if (listener != null) {
			mClickListener = listener;
			init();// update the click listener
		}
	}

	/**
	 * subclasses can use their own OnClickListener implementations to deal with
	 * the 'onClick' by default, it is a 'ButtonLongClickListener' instance
	 */
	protected void setNumButtonLongClickListener(OnLongClickListener listener) {
		if (listener != null) {
			mLongClickListener = listener;
			init();// update the longclick listener
		}
	}

	float pointX, pointY;

	public boolean dispatchTouchEvent(MotionEvent event) {
		pointX = event.getX();
		pointY = event.getY();
		return super.dispatchTouchEvent(event);
	}

	/**
	 * provide this method to facilitate some Widget's use, such as EditText,who
	 * receive user input through onKeyDown.
	 *
	 * @param c
	 * @return
	 */
	public static KeyEvent generateKeyEvent(char c) {

		int keycode;

		/*
		 * if (c == '*') keycode = KeyEvent.KEYCODE_STAR; else if (c == '#')
		 * keycode = KeyEvent.KEYCODE_POUND; else if (c >= '0' && c <= '9')
		 * keycode = KeyEvent.KEYCODE_0 + c - '0'; else throw new
		 * AssertionError("illeagal input on touch pad "+ c);
		 */

		switch (c) {
		case '*':
			keycode = KeyEvent.KEYCODE_STAR;
			break;
		case '#':
			keycode = KeyEvent.KEYCODE_POUND;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			keycode = KeyEvent.KEYCODE_0 + c - '0';
			break;
		case 'P':
			keycode = KeyEvent.KEYCODE_P;
			break;
		case 'W':
			keycode = KeyEvent.KEYCODE_W;
			break;
		case '+':
			keycode = KeyEvent.KEYCODE_PLUS;
			break;
		default:
			throw new AssertionError("illeagal input on touch pad " + c);
		}

		KeyEvent keyevent = new KeyEvent(KeyEvent.ACTION_DOWN, keycode);
		return keyevent;
	}
};
