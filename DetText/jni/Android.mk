LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include /home/vivek/OpenCV-2.4.6-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := DetText
LOCAL_SRC_FILES := DetText.cpp

include $(BUILD_SHARED_LIBRARY)
