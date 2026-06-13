#!/bin/bash
set -e

ndk-build -C jni/

mkdir -p build/out/lib/arm64-v8a

cp libs/arm64-v8a/libcymux.so build/out/lib/arm64-v8a/

python3 build/axml_encoder.py AndroidManifest.xml build/out/AndroidManifest.xml

cd build/out
zip -r ../../cymux_unaligned.apk AndroidManifest.xml lib/
cd ../..

zipalign -f -v 4 cymux_unaligned.apk cymux_aligned.apk
apksigner sign --ks build/debug.keystore --ks-key-alias androiddebugkey --ks-pass pass:android --min-sdk-version 31 --out cymux_production.apk cymux_aligned.apk

rm cymux_unaligned.apk cymux_aligned.apk
echo "SUCCESS: cymux_production.apk generated."
