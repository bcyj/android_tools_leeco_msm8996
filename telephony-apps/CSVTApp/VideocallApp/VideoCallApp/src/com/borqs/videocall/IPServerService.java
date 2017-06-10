/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.ServerSocket;
import java.net.Socket;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

public class IPServerService extends Service
{
    public static final int VT_SOCKET_LISTENER_STATUS = 1;

    Handler mHandler = null;

	private final static String TAG = "VT/IPServerService";

	final static String VT_IP_CONNECT_MSG = "Hello VT";
	final static String VT_IP_VALIDFAILED_MSG = "INVALID Hello Message";
	final static String VT_IP_CONNECT_BUSY_MSG = "BUSY";
	final static String VT_IP_CONNECT_OK_MSG = "OK";
	final static String VT_IP_CLOSED_MSG = "DISCONNECTING";
	final static String VT_IP_FALLBACK_MSG = "FALLBACK";

	final static int VT_LISTEN_PORT = 60000+1;

	final static String INTENT_CLOSE_CALLSCREEN="com.borqs.videocall.action.CloseCallScreen";
	final static String INTENT_ACCEPT_IPCALL="com.borqs.videocall.action.CloseCallScreen";

	//private int mRingingWaitMS = 50000; //MAX wait duration for ringing in MS units

	//enumeration values for service server-client interaction
	final static public int eNotifyType_Release = 0;
    final static public int eNotifyType_AcceptCall = 1;

	private static IPServerService sService = null;

	public class LocalBinder extends Binder {
		IPServerService getService(){
		return IPServerService.this;
		}
	}

	public IPServerService()
	{
	}

	public static interface OnMonitorNotifyListener
	{
		public void OnClose();
	}

	public static void startService(final Context context) {
		if (MyLog.DEBUG) MyLog.v(TAG, "startService Called.");

		if (sService != null) {
			if (MyLog.DEBUG) MyLog.v(TAG, "service existed alreay.");
			return;
		}

		if (MyLog.DEBUG) MyLog.v(TAG, "To startService.");

		context.startService(new Intent(context,
				IPServerService.class));

		return;
	}

	public static void stopService() {
		if (MyLog.DEBUG) MyLog.v(TAG, "stopService Called.");

        if (sService == null) {
		if (MyLog.DEBUG) MyLog.v(TAG, "nothing to stop.");
            return;
        }

		if( sService != null)
			sService.stopSelf();
        //sService = null;
        return;
    }

	@Override
	public void onCreate() {
		super.onCreate();
		//to bind
		if (MyLog.DEBUG) MyLog.v( TAG, "IpConnectService onCreate!");
		sService = this;
		InstallMonitor();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		if (MyLog.DEBUG) MyLog.v( TAG, "IpConnectService onDestroy");
		RemoveMonitor();
		sService = null;
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	// This is the object that receives interactions from clients.  See
	 // RemoteService for a more complete example.
	private final IBinder mBinder = new LocalBinder();

	private class MonitorServer extends Thread
	{
		private ServerSocket serverSocket;
		final Object lockA = new Object();

		//private volatile boolean mIsBusy = false;
		private final static int ACCEPT_RESULT_INIT = 0;
		private final static int ACCEPT_RESULT_ACCEPTED = 1;
		//private final static int ACCEPT_RESULT_REJECTED = 2;
		private final static int ACCEPT_RESULT_FALLBACK = 3;
		private final static int ACCEPT_RESULT_END = 4;

		private volatile boolean mIsKeepRunning = true;
		private volatile int mAcceptResult = ACCEPT_RESULT_INIT;

		public MonitorServer(int port)
		{
			try{
				serverSocket = new ServerSocket(port);
				serverSocket.setSoTimeout( SO_TIMEOUT);
			}
			catch( Exception e){
				if (MyLog.DEBUG) MyLog.d(TAG, "Failed to create serverSocket." + e.getMessage());
				//e.printStackTrace();
				sService.stopSelf();
			}
		}

		public void ExitService()
		{
			if (MyLog.DEBUG) MyLog.d(TAG, "Exit Service called.");
			try{
				synchronized( lockA){
					mIsKeepRunning = false;
					this.join();
					serverSocket.close();
				}
			}
			catch(IOException e) {
				if (MyLog.DEBUG) MyLog.d(TAG, "Monitor Service:ExitService IOException" + e.getMessage());
			}
			catch( Exception e1){
				if (MyLog.DEBUG) MyLog.d(TAG, "Monitor Service:General ExitService IOException" + e1.getMessage());
			}
			if (MyLog.DEBUG) MyLog.v(TAG, "Exit Service...");
		}

		private void UpdateAcceptResult( int nResult)
		{
			if (MyLog.DEBUG) MyLog.d(TAG, "Update Accept Result:" + nResult);
			if( ( this.isAlive() == false) || ( this.isInterrupted() == true)){
				if (MyLog.DEBUG) MyLog.d( TAG, "Update Accept Result, thread dead or interrupted, just return.");
				return;
			}

			synchronized( lockA){
				mAcceptResult = nResult;
				try{
					lockA.wait();
				}catch( InterruptedException e){
					if (MyLog.DEBUG) MyLog.d( TAG, "wait result." + e.getMessage());
				}
			}
			return;
		}

		//Request Respond Procedure
		private final static int RESPOND_OK = 0;
		private final static int RESPOND_INVALID_INPUT = 2;
		private final static int RESPOND_EXCEPTION =3;

		private final static int SO_TIMEOUT = 500; //500ms

		final private void ProcessRequest( Socket c, DataInputStream si, DataOutputStream so)
			throws InterruptedException, IOException
		{
			while( true){
				try{
					String str = si.readUTF();
					if (MyLog.DEBUG) MyLog.v(TAG, "Receive incoming message " + str);
					if( str != null){
						if( 0 != str.compareTo( VT_IP_CONNECT_MSG)){
							so.writeUTF( VT_IP_VALIDFAILED_MSG);
							so.flush();
							if (MyLog.DEBUG) MyLog.d(TAG, "Received a INvalid request, just return");
							return;
						}
						else{
							//To start the CallScreen and wait for Accept or Refuse
							if (MyLog.DEBUG) MyLog.v(TAG, "Received a valid request, To Start the InCallScreen");
							break;
						}
					}//end of read a non-null string
				}catch( InterruptedIOException e)  //time out exception
				{
					//noting, this should be the time-out exception
					//FIXME: remove below logs, it will fill the screen
					//if (MyLog.DEBUG) MyLog.d(TAG, "Exception while handle client request , " + e.getMessage());
				}
				if( mIsKeepRunning == false){
					if (MyLog.DEBUG) MyLog.d(TAG, "Quit handle request process because received thread exit signal.");
					return;
				}
			}//end of while

			mAcceptResult = ACCEPT_RESULT_INIT;
			String strInputURL = c.getRemoteSocketAddress().toString();
			strInputURL = strInputURL.substring(0, strInputURL.indexOf('/'));
			//Launch Call Screen
			VideoCallApp.getInstance().launchVTScreen( false, false,
					strInputURL, VideoCallApp.CALL_FROM_IP, mConnect);

			if (MyLog.DEBUG) MyLog.v(TAG, "Wait for accepting the answser IN ProcessRequest.");

			while( mIsKeepRunning){
				synchronized( lockA){
					if( mAcceptResult != ACCEPT_RESULT_INIT){
						if (MyLog.DEBUG) MyLog.d( TAG, "UI made decision.");
						if( mAcceptResult == ACCEPT_RESULT_ACCEPTED){
							if (MyLog.DEBUG) MyLog.d( TAG, "UI make decision, ACCEPTED ");
							//notify peer that make a deal
							//!!!!FIXME, a temporary solution, ensure server create before client connect
							//sleep(1000);
							so.writeUTF( VT_IP_CONNECT_OK_MSG);
							so.flush();
							mHandler.sendMessage( Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_CONNECTED));
							//reset the accept result
							mAcceptResult = ACCEPT_RESULT_INIT;
							lockA.notify();
						} else {
							switch( mAcceptResult)
							{
							case ACCEPT_RESULT_FALLBACK:
								if (MyLog.DEBUG) MyLog.d( TAG, "UI make decision, fallback.");
								so.writeUTF( VT_IP_FALLBACK_MSG);
								break;
									/*
								case ACCEPT_RESULT_REJECTED:
									if (MyLog.DEBUG) MyLog.d( TAG, "UI make decision, rejected.");
									so.writeUTF( VT_IP_CLOSED_MSG);
									break;
									*/
							case ACCEPT_RESULT_END:
								if (MyLog.DEBUG) MyLog.d( TAG, "UI make decision, end.");
								so.writeUTF( VT_IP_CLOSED_MSG);
								break;
							}//end of switch
							so.flush();
							lockA.notify();
							break;  //exit the loop
						}
					}
				}//end of synchronized lockA

				//check if the client make disconnection actively
				try{
					String str = si.readUTF();
					if (MyLog.DEBUG) MyLog.d(TAG, "Waiting...Receive incoming message " + str);
					if( 0 == str.compareTo( VT_IP_CLOSED_MSG)){  //closed
						if (MyLog.DEBUG) MyLog.d(TAG, "OK. say good bye");
						mHandler.sendMessage( Message.obtain( mHandler, IVTConnection.VTCALL_RESULT_DISCONNECTED));
						break;
					}
					else{   //debug purpose: unwanted client message
						if (MyLog.DEBUG) MyLog.d(TAG, "read unwanted message: " + str);
					}
				}catch( InterruptedIOException e)  //time out exception
				{
					//noting, this should be the time-out exception
					//FIXME: remove below logs, it will fill the screen
					//if (MyLog.DEBUG) MyLog.d(TAG, "Exception while reading client, " + e.getMessage());
				}
			} //end of while loop

			return;
		}

		//Main run loop
		public void run()
		{
			if (MyLog.DEBUG) MyLog.v(TAG, "Enter monitor thread run process...");
			Socket c = null;
			DataInputStream si = null;
			DataOutputStream so = null;

			while( mIsKeepRunning){
				try{
					try{
						c = serverSocket.accept();
						if (MyLog.DEBUG) MyLog.v(TAG, "NEW accept connect from" + c.getRemoteSocketAddress());

						c.setSoTimeout( SO_TIMEOUT);
						si = new DataInputStream( c.getInputStream());
						so = new DataOutputStream( c.getOutputStream());
					}catch( InterruptedIOException e)  //time out exception
					{
						//noting, this should be the time-out exception
						//FIXME: remove below logs, it will fill the screen
						//if (MyLog.DEBUG) MyLog.d(TAG, "Exception while Acceptinging client, " + e.getMessage());
						continue;
					}
					catch( Exception e){
						if (MyLog.DEBUG) MyLog.d(TAG, "General Exception while Accepting client, " + e.getMessage());
						continue;
					}

					ProcessRequest( c, si, so);
					c.close();
				}
				catch(Exception e){
					if (MyLog.DEBUG) MyLog.v(TAG, "Monitor Service:Run Exception" + e.getMessage());
				}
			}//end of while Loop
			if (MyLog.DEBUG) MyLog.v(TAG, "Service monitor thread existing...");
		}

	}//end of MonitorServer class definition

	private MonitorServer mMonitor;

	private IVTConnection mConnect = new IVTConnection(){

		public void acceptCall(){
			if (MyLog.DEBUG) MyLog.d( TAG, "acceptCall...");
			mMonitor.UpdateAcceptResult( MonitorServer.ACCEPT_RESULT_ACCEPTED);
		}
		public void endSession(){
			if (MyLog.DEBUG) MyLog.d(TAG, "end Session...");
			mMonitor.UpdateAcceptResult( MonitorServer.ACCEPT_RESULT_END);
		}
		public void rejectSession(){
			if (MyLog.DEBUG) MyLog.d(TAG, "end Session...");
			mMonitor.UpdateAcceptResult( MonitorServer.ACCEPT_RESULT_END);
		}
		public void fallBack(){
			if (MyLog.DEBUG) MyLog.d(TAG, "fallBack...");
			mMonitor.UpdateAcceptResult( MonitorServer.ACCEPT_RESULT_FALLBACK);
		}

		public void setHandler( Handler h){
			if (MyLog.DEBUG) MyLog.d(TAG, "setHandler");
			mHandler = h;
		}

		public void clear(){
			return;
		}
	};

	//used in server side
	/*
	 * this function will be called in OnCreate
	 */
	private void InstallMonitor()
	{
		mMonitor = new MonitorServer( VT_LISTEN_PORT);
		mMonitor.start();
	}

	/*
	 * this function will be called in OnDestroy
	 */
	private void RemoveMonitor()
	{
		if (MyLog.DEBUG) MyLog.v( TAG, "Close Notification, to shut down the thread.!");
		mMonitor.ExitService();
		mMonitor = null;
	}

}
