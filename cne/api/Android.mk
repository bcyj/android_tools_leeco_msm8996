LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := cne/inc
LOCAL_COPY_HEADERS := CneFactory.h
LOCAL_COPY_HEADERS += ICneBatteryObserver.h
LOCAL_COPY_HEADERS += ICneBatteryObserverNotifier.h
LOCAL_COPY_HEADERS += ICneNetworkObserver.h
LOCAL_COPY_HEADERS += ICneNetworkObserverNotifier.h
LOCAL_COPY_HEADERS += ICneFeatureObserver.h
LOCAL_COPY_HEADERS += ICneFeatureObserverNotifier.h
LOCAL_COPY_HEADERS += ICneObserverDefs.h

include $(BUILD_COPY_HEADERS)
