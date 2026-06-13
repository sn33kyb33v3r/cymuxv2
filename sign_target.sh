#!/bin/bash
set -e

KEYTOOL_BIN=$(which keytool || find /usr/lib/jvm/ -name keytool -type f | head -n 1)
APKSIGNER_BIN=$(which apksigner || find /usr/lib/android-sdk/ -name apksigner -type f | head -n 1)

if [ -z "$KEYTOOL_BIN" ]; then
    echo "[ERROR] Missing system Java utility vectors"
    exit 1
fi

if [ ! -f cymux_debug.keystore ]; then
    "$KEYTOOL_BIN" -genkey -v \
        -keystore cymux_debug.keystore \
        -alias cymux_dev_key \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass cymuxpassword \
        -keypass cymuxpassword \
        -dname "CN=cymux.core, O=Cymux, C=US"
fi

if [ -n "$APKSIGNER_BIN" ]; then
    # Inject validation flags to explicitly force signature boundaries without structural scanning
    "$APKSIGNER_BIN" sign \
        --ks cymux_debug.keystore \
        --ks-pass pass:cymuxpassword \
        --key-pass pass:cymuxpassword \
        --min-sdk-version 31 \
        --max-sdk-version 31 \
        --out cymux_production.apk \
        cymux_server.apk
    echo "Production manifest signed: cymux_production.apk"
else
    JARSIGNER_BIN=$(which jarsigner || find /usr/lib/jvm/ -name jarsigner -type f | head -n 1)
    if [ -n "$JARSIGNER_BIN" ]; then
        cp cymux_server.apk cymux_production.apk
        "$JARSIGNER_BIN" -keystore cymux_debug.keystore \
            -storepass cymuxpassword \
            -keypass cymuxpassword \
            cymux_production.apk cymux_dev_key
        echo "Jar verification signatures injected: cymux_production.apk"
    else
        echo "[ERROR] No viable signature injection binaries detected."
        exit 1
    fi
fi
