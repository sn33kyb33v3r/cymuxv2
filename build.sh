#!/bin/bash
set -e

export NDK_PROJECT_PATH=.
./android-ndk-r25c/ndk-build NDK_APPLICATION_MK=jni/Application.mk

mkdir -p build/apk/lib/arm64-v8a
cp libs/arm64-v8a/libcymux_core.so build/apk/lib/arm64-v8a/

echo "Native compilation pass complete."
