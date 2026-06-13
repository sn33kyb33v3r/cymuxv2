LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := cymux
LOCAL_SRC_FILES := src/core/topology.c src/core/input_queue.c src/core/transport.c
LOCAL_LDLIBS    := -llog -landroid
include $(BUILD_SHARED_LIBRARY)
