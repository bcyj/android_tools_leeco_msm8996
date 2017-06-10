/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package android.nfc.dta;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.nfc.dta.IDtaHelper;
import android.nfc.NdefMessage;
import android.os.RemoteException;
import android.os.IBinder;
import android.util.Log;

// <DTA>
// Helper class for DTA functionalities.
public final class DtaHelper {
    static final String TAG = "DtaHelper";
    private IDtaHelper mService;
    private Context mContext;

    public class MyServiceConnection implements ServiceConnection {

        private boolean connected = false;
        private Object lock = new Object();

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            connected = true;

            synchronized (lock) {
                lock.notifyAll();
            }
            mService = IDtaHelper.Stub.asInterface(binder);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            connected = false;
            mService = null;
        }

        public void waitUntilConnected() throws InterruptedException {
            if (!connected) {
                synchronized (lock) {
                    lock.wait();
                }
            }
        }

    }

    private MyServiceConnection mConnection = new MyServiceConnection();

    private void initService() {
        Intent intent = new Intent(IDtaHelper.class.getName());
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        try {
            mConnection.waitUntilConnected();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public DtaHelper(Context context) {
        mService = null;
        mContext = context;
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.WRITE_SECURE_SETTINGS, "WRITE_SECURE_SETTINGS permission required");
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.NFC, "NFC permission required");
        initService();
    }

    public void dta_set_pattern_number(int pattern) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] dta_set_pattern_number");
            mService.dta_set_pattern_number(pattern);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }


    public int dta_get_pattern_number() {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] dta_get_pattern_number");
            return mService.dta_get_pattern_number();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        } catch(Exception e) {
            e.printStackTrace();
            return -1;
        }

    }

    public boolean in_dta_mode() {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] in_dta_mode");
            return mService.in_dta_mode();
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }

    }

    public String get_text_from_ndef(NdefMessage ndefMessage) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] get_text_from_ndef");
            return mService.get_text_from_ndef(ndefMessage);
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        } catch(Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public int snep_client_create(String serviceName) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_client_create: " + serviceName);
            return mService.snep_client_create(serviceName);
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        } catch(Exception e) {
            e.printStackTrace();
            return -1;
        }
    }

    public boolean snep_client_connect(int handle) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_client_connect: " + handle);
            return mService.snep_client_connect(handle);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean snep_client_put(int handle, NdefMessage ndefMessage) {
        try {
            Log.d(TAG, "[DTA] snep_client_put: " + handle);
            return mService.snep_client_put(handle, ndefMessage);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public NdefMessage snep_client_get(int handle, NdefMessage ndefMessage) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_client_get: " + handle);
            return mService.snep_client_get(handle, ndefMessage);
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        } catch(Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public void snep_client_close(int handle) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_client_close: " + handle);
            mService.snep_client_close(handle);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }


    public int snep_server_create(String serviceName, boolean enableExtendedDTAServer) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_server_create: " + serviceName);
            return mService.snep_server_create(serviceName, enableExtendedDTAServer);
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        } catch(Exception e) {
            e.printStackTrace();
            return -1;
        }
    }

    public void snep_server_close(int handle) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] snep_server_close: " + handle);
            mService.snep_server_close(handle);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * Deactivate NFC connection
     *
     * @param deactivationType Integer value that determines the deactivation type:
     * 1 = NFC-DEP DSL request
     * 2 = NFC-DEP RLS request
     * 3 = General deactivation to sleep mode
     * 4 = General deactivation
     * @return true if deactivate command is passed to lower level for sending but
     * does not quarantee actual sending even though it's most likely
     */

    public boolean nfcDeactivate(int deactivationType) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] nfcDeactivate()");
            return mService.nfcDeactivate(deactivationType);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * Starts a connection-oriented echo service including a server
     * listening for incoming messages and a client for sending messages
     * received by the server.
     *
     * @param serviceNameIn service name (URN) of the connection-oriented inbound service
     * @param serviceNameOut service name (URN) of the connection-oriented outbound service
     */
    public void startLlcpCoEchoServer(String serviceNameIn, String serviceNameOut) {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] startLlcpCoEchoServer");
            mService.startLlcpCoEchoServer(serviceNameIn, serviceNameOut);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Stops a running connection-oriented echo service.
     */
    public void stopLlcpCoEchoServer() {
        if(mService == null)
            initService();
        try {
            Log.d(TAG, "[DTA] stopLlcpCoEchoServer");
            mService.stopLlcpCoEchoServer();
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * Starts a connectionless echo service including a server listening for incoming messages
     * and a client for sending messages received by the server.
     *
     * @param serviceNameIn service name (URN) of the connectionless inbound service
     * @param serviceNameOut service name (URN) of the connectionless outbound service
     */
    public void startLlcpClEchoServer(String serviceNameIn, String serviceNameOut) {
        try {
            Log.d(TAG, "[DTA] startLlcpClEchoServer");
            mService.startLlcpClEchoServer(serviceNameIn, serviceNameOut);
        }  catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    /**
     * Stops a running connectionless echo service.
     */
    public void stopLlcpClEchoServer() {
        try {
            Log.d(TAG, "[DTA] stopLlcpClEchoServer");
            mService.stopLlcpClEchoServer();
        }  catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    /**
     * Static check functions
     */
    public static boolean isInDtaMode() {
        try {
            int patternNumber = Integer.parseInt(System.getProperty("sys.dtapattern"));
            return (patternNumber >= 0x0000);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isInLlcpOrSnepMode() {
        try {
            int patternNumber = Integer.parseInt(System.getProperty("sys.dtapattern"));
            return (patternNumber >= 4608);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isInLlcpSap() {
        try {
            int patternNumber = Integer.parseInt(System.getProperty("sys.dtapattern"));
            return (patternNumber == 4608);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isInLlcpName() {
        try {
            int patternNumber = Integer.parseInt(System.getProperty("sys.dtapattern"));
            return (patternNumber == 4672);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isInLlcpSnl() {
        try {
            int patternNumber = Integer.parseInt(System.getProperty("sys.dtapattern"));
            return (patternNumber == 4736);
        } catch (Exception e) {
            return false;
        }
    }
}
// </DTA>
