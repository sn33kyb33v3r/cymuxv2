LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cymux_core

LOCAL_SRC_FILES := \
    src/core/main_activity.c \
    src/core/input_bridge.c \
    src/core/cymux_engine.c \
    src/core/pty_handler.c \
    src/core/overlay_window.c \
    src/parser/ansi_simd.c \
    src/fs/mirror_tree.c \
    src/fs/inotify_loop.c \
    src/fs/atomic_operations.c \
    src/graphics/font_atlas.c \
    src/graphics/renderer.c

LOCAL_LDLIBS := -llog -landroid -lGLESv3 -lEGL
LOCAL_CFLAGS := -O3 -march=armv8-a+simd -flto -D_GNU_SOURCE -DANDROID_NDK_BUILD
LOCAL_LDFLAGS := -flto

include $(BUILD_SHARED_LIBRARY)
