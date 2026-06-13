#!/bin/bash
set -e

# Core deployment directory verification
APK_DIR="build/apk"
mkdir -p "${APK_DIR}/res/values"

cat << 'INNER_EOF' > "${APK_DIR}/res/values/strings.xml"
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">CymuxV2</string>
</resources>
INNER_EOF

# Assemble explicit target manifest
cp AndroidManifest.xml "${APK_DIR}/"

echo "APK packaging directories structured."
