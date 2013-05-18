LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := myndk
LOCAL_SRC_FILES := SLIC.cpp graph.cpp maxflow.cpp GMM.cpp grabcut.cpp myndk.cpp
LOCAL_LDLIBS	:= -lm -llog
include $(BUILD_SHARED_LIBRARY)