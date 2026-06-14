LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := cymux
LOCAL_SRC_FILES := src/core/topology.c src/core/input_queue.c src/core/transport.c src/core/parser.c src/core/renderer.c src/core/pty.c src/core/jni_bridge.c
LOCAL_LDLIBS    := -llog -landroid -lGLESv3
LOCAL_CFLAGS    := -O3 -march=armv8-a+simd
include $(BUILD_SHARED_LIBRARY)
