/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.nfc;

import java.lang.reflect.Proxy;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import android.util.Log;
import java.util.LinkedList;
import java.util.ArrayList;
import android.content.Context;
import android.os.ServiceManager;
import android.os.Message;
import android.app.Application;

import com.android.nfc.NfcService;
import com.android.nfc.DeviceHost;
import com.android.nfc.DeviceHost.DeviceHostListener;
import com.android.nfc.dhimpl.NativeNfcManager;
import com.android.nfc.NfceeAccessControl;

import qcom.nfc.IQNfcExtras;
import qcom.nfc.IQNfcSecureElementManager;
import com.android.nfc.QSecureElementManager;
import com.android.nfc.QHostListener;

public class QNfcService extends IQNfcExtras.Stub{

    private Context mContext;
    private final static String TAG = "QNfcService";
    private final boolean DBG = false;
    private QHostListener mQHostListener;
    private DeviceHostListener mDeviceHostListener;
    private DeviceHost mDeviceHost;
    private static NfcService mNfcService;
    private static QNfcService mQNfcService;
    private QNfcServiceHandler mHandler;
    private QSecureElementManager mSecureElementManager;

    public QNfcService(Application application)
    {
        mQNfcService = this;
        mContext = application;
        mHandler = new QNfcServiceHandler();

        Class<?>[] interfaces = new Class<?>[1];
        interfaces[0]=DeviceHostListener.class;
        NfcServiceDeviceHostListenerProxyHandler hostListener = new NfcServiceDeviceHostListenerProxyHandler();
        mDeviceHostListener = (DeviceHostListener)
                     Proxy.newProxyInstance(
                       DeviceHostListener.class.getClassLoader(),  interfaces,
                       hostListener);

        mQHostListener = new simpleQHostListener();
        NativeNfcManager.registerQHostListener(mQHostListener);
        mDeviceHost = new NativeNfcManager(mContext, mDeviceHostListener);
        NfcService mNfcService = new NfcService(application, mDeviceHost,mHandler);
        hostListener.addListener(mNfcService);
        hostListener.flush();
        ServiceManager.addService(mNfcService.SERVICE_NAME,mNfcService.mNfcAdapter);
        mSecureElementManager = new QSecureElementManager(application, mDeviceHost, mNfcService);
        mHandler.addCallback(mSecureElementManager.getCallback());
        //todo: make a proxy adapater so we can free the nfc service.
        //todo: move the dta helper service here
        //todo: implement the dta tag service as a proxy service to reduce the amount of coppied code.
        //ServiceManager.addService("QNFC",this);
    }

    private class simpleQHostListener implements QHostListener{
        public void onCardEmulationAidSelected(byte[] dataBuf) {
            if(DBG) Log.d(TAG,"onCardEmulationAidSelected()");
            mSecureElementManager.onCardEmulationAidSelected(dataBuf);
        }
        public void onRfInterfaceDeactivated() {
            if(DBG) Log.d(TAG,"onRfInterfaceDeactivated()");
            mSecureElementManager.onRfInterfaceDeactivated();
        }
    }


    final static private class NfcServiceDeviceHostListenerProxyHandler implements InvocationHandler {
        private DeviceHostListener mHostListener;
        private LinkedList<Invocation> delayedInvocations;
        private static final class Invocation {
            final Object proxy;
            final Method method;
            final Object[] args;
            public Invocation( Object proxy, Method method, Object[] args){
                this.proxy  = proxy;
                this.method = method;
                this.args   = args;
            }
        }
        public NfcServiceDeviceHostListenerProxyHandler(){
            delayedInvocations = new LinkedList<Invocation>();
        }

        public Object invoke(Object proxy, Method method, Object[] args) {
            try {
                if(mHostListener != null)
                    return method.invoke(mHostListener, args);
                else {
                    Log.d(TAG, "invoking " + method + " on proxy; returning null and waiting for initialization");
                    delayedInvocations.add(new Invocation(proxy, method, args));
                    return null;
                }
            } catch (IllegalAccessException | IllegalArgumentException
                    | InvocationTargetException e) {
                e.printStackTrace();
                return null;
            }
        }

        public void addListener(DeviceHostListener listener) {
            if(mHostListener == null) mHostListener = listener;
        }

        public void flush() {
            for(Invocation i : delayedInvocations) {
                Log.d(TAG, "invoking " + i.method + " in flush action");
                invoke(i.proxy, i.method, i.args);
            }
            delayedInvocations=null;
        }
    }

    public static abstract class HandlerCallback{
        private QNfcServiceHandler mHandler=null;
        private int what = -1;

        void sendMessage(Message msg)
        {
            Message encapsulatingMessage= new Message();
            encapsulatingMessage.what = what;
            encapsulatingMessage.obj = msg;
            mHandler.sendMessage(encapsulatingMessage);
        }

        protected abstract void handleMessage(Message msg);
    }

    private final static class QNfcServiceHandler extends NfcService.NfcServiceExtentionHandler
    {
        private ArrayList<HandlerCallback> mRegisteredHandlers;
        QNfcServiceHandler() {
            mRegisteredHandlers = new ArrayList<HandlerCallback>();
        }
        protected void handleMessage(Message msg)
        {
            if((0<=msg.what) && (msg.what<mRegisteredHandlers.size()))
                mRegisteredHandlers.get(msg.what).handleMessage((Message)msg.obj);
        }

        public synchronized void addCallback(HandlerCallback cbk){
            if(cbk==null) throw new NullPointerException("attempted to regsiter a null handler");
            cbk.mHandler = this;
            cbk.what = mRegisteredHandlers.size();
            mRegisteredHandlers.add(cbk);
        }
    }

    public static QNfcService getInstance()
    {
        return mQNfcService;
    }

    public IQNfcSecureElementManager.Stub getSecureElementManagerStub(){
       return mSecureElementManager;
    }


/*
    final static class QNfcSecureElementService extends IQNfcSecureElement.stub {

    }
*/
}
