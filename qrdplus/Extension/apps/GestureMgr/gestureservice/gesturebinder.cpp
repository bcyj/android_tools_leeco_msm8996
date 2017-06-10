/******************************************************************************

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/
#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>

using namespace android;

enum {
	ON_TOUCH = 1, ON_TOUCH_DOWN, ON_TOUCH_UP
};

static sp<IBinder> getServiceManager() {
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> gesturebinder;
	while ((gesturebinder = sm->getService(String16("service.gesture"))) == 0) {
		usleep(1000000);
	}
	return gesturebinder;
}

#ifdef __cplusplus
extern "C" void notifyTouch(int x, int y)
#else
		void notifyTouch(int x, int y)
#endif
		{
	Parcel data, reply;
	data.writeInterfaceToken(
			String16("com.qualcomm.qti.gesture.IGestureManager"));
	data.writeInt32(x);
	data.writeInt32(y);
	getServiceManager()->transact(ON_TOUCH, data, &reply);
}

#ifdef __cplusplus
extern "C" void notifyTouchDown()
#else
void notifyTouchDown()
#endif
{
	Parcel data, reply;
	data.writeInterfaceToken(
			String16("com.qualcomm.qti.gesture.IGestureManager"));
	getServiceManager()->transact(ON_TOUCH_DOWN, data, &reply);
}

#ifdef __cplusplus
extern "C" void notifyTouchUp()
#else
void notifyTouchUp()
#endif
{
	Parcel data, reply;
	data.writeInterfaceToken(
			String16("com.qualcomm.qti.gesture.IGestureManager"));
	getServiceManager()->transact(ON_TOUCH_UP, data, &reply);
}
