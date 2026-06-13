LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cymux
LOCAL_SRC_FILES := src/core/main_activity.c \
                   src/core/input_bridge.c \
                   src/core/pty_handler.c \
                   src/core/io_loop.c \
                   src/parser/ansi_simd.c \
                   src/graphics/renderer.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/core

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3
LOCAL_CFLAGS := -O3 -march=armv8-a -mtune=cortex-a72 -flto -D_GNU_SOURCE

include $(BUILD_SHARED_LIBRARY)
