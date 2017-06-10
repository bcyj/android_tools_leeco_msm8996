/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.util.List;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
//import com.android.internal.telephony.AdnRecord;

import android.util.Config;
import android.util.Log;

public class FDNChecker {

	/*enum FDNCHECK_RESULT {
		FDNCHECK_QUERYING,
		FDNCHECK_ALLOWED,
		FDNCHECK_FORBIDDEN
	}*/
	//TODO here should we use enum or just static int value?
	//public static final int FDNCHECK_QUERYING = 0;
	//public static final int FDNCHECK_ALLOWED = 1;
//	public static final int FDNCHECK_FORBIDDEN = 2;
	final static String TAG = "VT/FDNChecker";
	private  static final String[] COLUMN_NAMES = new String[] {
			"name",
		"number"
	};

	 protected static final int QUERY_TOKEN = 0;
	 private static Uri mUri = Uri.parse("content://icc/fdn");

	private Handler mReceiver; // the one receives the check result
	private int mMsgAllowedCode = -1;
	private int mMsgForbiddenCode = -1;

	private Cursor mCursor;

	private QueryHandler mQueryHandler = new QueryHandler(VideoCallApp.getInstance().getContentResolver());

	private String mDialNumber; //the number to be checked

	private static FDNChecker mInstance;

	public static FDNChecker getInstance() {
		if (mInstance == null) {
			mInstance = new FDNChecker();
		}
		return mInstance;
	}

	//TODO object of this class should not be instaniated frequently
	/*public FDNChecker() {
		mReceiver = receiver;
		mContext = context;
		mQueryHandler = new QueryHandler(mContext.getContentResolver());
	}
	*/

	int startFDNCheck(Handler receiver,int msg_allowed_code,int msg_forbidden_code,String number) {
		mDialNumber = number;
		mReceiver = receiver;
		mMsgAllowedCode  = msg_allowed_code;
		mMsgForbiddenCode = msg_forbidden_code;

		log("launch thread to query fdn entries firstly");
		startQuery();
		return 0;
	}

	private void startQuery() {
		mQueryHandler.startQuery(QUERY_TOKEN, null, mUri, COLUMN_NAMES, null,
                null, null);
	}

	/**
	 *  do the match and notify the 'mReceiver' about the check result through a Message
	 */
	private void onQueryDone() {
		if (isFDNEntry()) {
			log("fdn matched, notify mReceiver");
			notifyResult(mMsgAllowedCode);
		} else {
			log("not match fdn, notify mReceiver");
			notifyResult(mMsgForbiddenCode);
		}
	}

	/**
	 * match the 'mDialNumber' against the entries in the 'mCursor'
	 * @return true means matched, otherwise return false
	 */
	private boolean isFDNEntry() {
		if (mCursor == null || mCursor.getCount() == 0) {
			log("fdn entry not set, forbidden");
			 //notifyResult(FDNCHECK_FORBIDDEN);
			return false;
		} else {
			//iterate all the rows of the 'mCursor' to see if one of them contains number that match the 'mDialNumber'
			int numberIdx = -1;
			try {
				numberIdx = mCursor.getColumnIndexOrThrow(COLUMN_NAMES[1]);
			} catch (IllegalArgumentException e) {
				log("fail to get column index for number:" + e);
				return false;
			}

			while (mCursor.moveToNext()) {
				String fdnNumber = mCursor.getString(numberIdx);
				if (fdnNumber == null)
					continue;
				if (matchedFDNNumber(mDialNumber,fdnNumber) ){
					log("match fdn number:" + fdnNumber);
				return true;
				}
			}

			//all the fdn entry has been scanned, none of them match the dial number
		return false;
		}
	}

	/**
     * the match rule is like this:
     * 1. Characters of a dialed number is matched with characters of a FDN number from left to right,
     * until all the chars of fdn number has been scanned; fdn number can be a substring of
     * dialed number
		2. Each wild character in fdn number can replace a character(0-9,*,#,+)
		3. pause char 'p' or 'P' in dialed number matched the char ',' in fdn number
     *
     * @param dialNumber user input number to be dialed out
     * @param fdnNumber the number stored in one fdn entry
     * @return true if dialNumber matches the fdnNumber, otherwise return false
     */
    private   boolean matchedFDNNumber(String dialNumber, String fdnNumber) {
	int len1 = dialNumber.length();
	int len2  = fdnNumber.length();

	int digit;
	for(int k = 0; k < len1; k++) {
	    digit = dialNumber.charAt(k);
		if(digit == 'p' || digit == 'P' || digit == 'w' || digit == 'W') {
			len1 = k;
			break;
		}
	}

	if (len1 < len2) {
		return false;
	}

	/*refer to PhoneNumberUtils.calledPartyBCDToString method, a fdn number can
	 contain following char :
	  +
	  0-9
	   *
	   #
	    N //wide char,it can match any one char
	    , //pause char, displayed as 'p' or 'P'
	    */
	int i = 0; //the input number index
	int j = 0; //the fdn number index
	for(; i < len1 && j < len2; i++,j++) {
		char c1 = dialNumber.charAt(i);
		char c2 = fdnNumber.charAt(j);
		if ( c1 == c2
				    ||  c2 == 'N' //wild char
				    ||  ((c1 == 'p' || c1 == 'P' ) && c2 == ',') ) {
			continue;
		} else  {
			return false;
		}
	}
	//reached here, means all the char in the fdn number has been matched
	if ( j == len2  && i <= len1)
		return true;
	else
		return false;
    }

	// notify the 'mReceiver' about the check result through a 'Message'
	private void notifyResult(int code) {
		if (mReceiver != null) {
		Message msg = Message.obtain(mReceiver, code);
		msg.obj = mDialNumber;
		msg.sendToTarget();
		} else {
			log("null mReceiver, not sending out result");
		}
		return;
	}

	private void log(String msg) {
		//if(PhoneApp.isPhoneLogEnabled || Config.LOGD) {
		  //  if (MyLog.DEBUG) MyLog.d( TAG, msg);
		//}

		if (MyLog.DEBUG) MyLog.d( TAG, msg);
	}

	private class QueryHandler extends AsyncQueryHandler {

        public QueryHandler(ContentResolver cr) {
            super(cr);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor c) {
            if(c != null){
                log("onQueryComplete: cursor.count=" + c.getCount());
                mCursor = c;
                onQueryDone();
            }else{
                log("onQueryComplete: cursor is null");
            }
        }
	}
}
