/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.InetSocketAddress;
import java.net.Socket;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.util.Log;

import org.codeaurora.ims.csvt.ICsvtService;

class VTCallProxy{

	final String TAG = "VT/VTCallProxy";
	final private static int CALL_CONNECT_TIMEOUT=60000;

	private interface CallClient{
		public void EndCall();
		public boolean StartCall();
	}

	private String mRemoteName;
	private Handler mHandler;
	private Context mCtx;

	CallClient mCallClient = null;

	private class IPCallClient extends Thread implements CallClient
	{
		private Socket mSocket = null;
		final Object lockA = new Object();
		private final static int CONNECT_RESULT_INIT = 0;
		private final static int CONNECT_RESULT_EXITING = 1;
		private final static int CONNECT_RESULT_END = 2;
		private int mConnectResult = CONNECT_RESULT_INIT;
		private final static int SO_TIMEOUT = 500; //500ms

		public IPCallClient(){
			if (MyLog.DEBUG) MyLog.v(TAG, "IPCallClient Construct.");
		};

		public boolean StartCall(){
			if (MyLog.DEBUG) MyLog.v(TAG, "IPCallClient StartCall.");
            start();
			return true;
		}

		public void EndCall(){
			if (MyLog.DEBUG) MyLog.d(TAG, "Call is being cancelled-->Interrupt.");
			synchronized( lockA){
				if( mConnectResult != CONNECT_RESULT_INIT){
					if (MyLog.DEBUG) MyLog.d(TAG, "cancel end call because connect thread exiting.");
					return;
				}

				mConnectResult = CONNECT_RESULT_END;
				try{
					lockA.wait();
				}catch( InterruptedException e){
					if (MyLog.DEBUG) MyLog.d(TAG, "Ip end call wait exception.");
				}
			}
			if (MyLog.DEBUG) MyLog.d(TAG, "ip EndCall end....");
		}

		final private void HandShake() throws IOException
		{
			if (MyLog.DEBUG) MyLog.v(TAG, "Call to hand shake.");

			mSocket.setSoTimeout( SO_TIMEOUT);
			DataInputStream si = new DataInputStream( mSocket.getInputStream());
			DataOutputStream so = new DataOutputStream( mSocket.getOutputStream());
			//to write the hello string to server
			so.writeUTF( IPServerService.VT_IP_CONNECT_MSG );
			so.flush();

			mConnectResult = CONNECT_RESULT_INIT;

			if (MyLog.DEBUG) MyLog.v(TAG, "To communicate with the responds from server.\n");

			String strResult = null;
			Message msg = null;
			boolean bGotValidMsg = false;
			while( true){

				try{
					bGotValidMsg = false;
					strResult = si.readUTF();
					bGotValidMsg = true;
				}catch(InterruptedIOException e)  //time out exception
				{
					//noting, this should be the time-out exception
					//FIXME: remove below logs, it will fill the screen
					//if (MyLog.DEBUG) MyLog.d(TAG, "Exception while reading client, " + e.getMessage());
				}

				synchronized( lockA){
					if( bGotValidMsg && ( strResult != null)){
						if (MyLog.DEBUG) MyLog.d(TAG, "Receives responds from server: " + strResult);

						if( strResult.compareTo( IPServerService.VT_IP_CONNECT_OK_MSG) == 0){
							msg = Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_CONNECTED);
						}
						else if( strResult.compareTo( IPServerService.VT_IP_CLOSED_MSG) == 0){
							msg = Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_DISCONNECTED);
							mConnectResult = CONNECT_RESULT_EXITING;
						}
						else if( strResult.compareTo( IPServerService.VT_IP_FALLBACK_MSG) == 0){
							msg = Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_FALLBACK_88);
							mConnectResult = CONNECT_RESULT_EXITING;
						}
					}

					if( mConnectResult == CONNECT_RESULT_END){
						if (MyLog.DEBUG) MyLog.d( TAG, "end call session.");
						so.writeUTF( IPServerService.VT_IP_CLOSED_MSG);
						so.flush();
						lockA.notify();
						break;
					}

					if( bGotValidMsg){
						mHandler.sendMessage( msg);
						if( mConnectResult != CONNECT_RESULT_INIT){
							if (MyLog.DEBUG) MyLog.d(TAG, "receive exit instructment, exit, result: " + mConnectResult);
							break;
						}
					}
				}
			} //end of while loop

			return;
		}

		@Override
		public void run()
		{
			if (MyLog.DEBUG) MyLog.v(TAG, "Call thread run process...");
			try{
				mSocket = new Socket();
				mSocket.connect(
						new InetSocketAddress( mRemoteName, IPServerService.VT_LISTEN_PORT),
						CALL_CONNECT_TIMEOUT);
				HandShake();
				mSocket.close();
				mSocket = null;
			}
			catch( Exception e)
			{
				mHandler.sendMessage( Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_DISCONNECTED));
				if (MyLog.DEBUG) MyLog.v(TAG, "Call Thread Exception on Socket: " + e.getMessage());
			}
			if (MyLog.DEBUG) MyLog.v(TAG, "Call Thread is existing...");
		}
	}; //end of subclass IPConnectClient

        private class TTYCallClient implements CallClient{
        private VTPhoneConnection mVTTelephony = null;
            TTYCallClient(){
                if (MyLog.DEBUG) MyLog.v(TAG, "TTYCallClient created");
                //FIXME: to bind with vt service and get IVideoTelephony interface
                Context ctx = VTCallProxy.this.mCtx;
                mVTTelephony = new VTPhoneConnection(ctx, VTCallUtils.getCsvtService());
                mVTTelephony.setHandler( VTCallProxy.this.mHandler);
             }

		//this should be call from client
		public void EndCall(){

			if( mVTTelephony != null){
				mVTTelephony.endSession();
				mVTTelephony.clear();
				mVTTelephony = null;
			}
		}

		public boolean StartCall(){
			if (MyLog.DEBUG) MyLog.v( TAG, "TTYCall Call thread run process..., " + mRemoteName);
			if( mVTTelephony != null)
				return mVTTelephony.call( mRemoteName);
			return false;
		}
	} //end of subclass TTYCallClient

	//class VTCallProxy
	VTCallProxy( String remoteName,
			Handler h,
			Context ctx){
		mRemoteName = remoteName;
		mHandler = h;
		mCallClient = null;
		mCtx = ctx;
	}

	boolean startCall( int nMode){
		if (MyLog.DEBUG) MyLog.d(TAG, "start Call");

		if( mCallClient != null){
			Log.w(TAG, "Call client existed already.");
			return false;
		}

		if (MyLog.DEBUG) MyLog.d(TAG, "startCall, mode: " + nMode);

		switch(nMode){
		case VideoCallApp.CALL_FROM_IP:
			mCallClient = new IPCallClient();
			break;
		case VideoCallApp.CALL_FROM_TTY:
			mCallClient = new TTYCallClient();
			break;
		default:
			Log.w(TAG, "Unknow dial mode." + nMode);
			return false;
		}
		if (MyLog.DEBUG) MyLog.d(TAG, "start call thread...");

		return mCallClient.StartCall();
	}

	void endCall(){
		if (MyLog.DEBUG) MyLog.v( TAG, "end call notification.");
		if( mCallClient != null){
			mCallClient.EndCall();
		}
	}
}
