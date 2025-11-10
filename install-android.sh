#!/bin/bash
echo "📱 Android Installation Helper"

# بررسی اتصال دستگاه
echo "🔍 Checking device connection..."
adb devices

# فعال کردن منابع ناشناس
echo "🔓 Enabling unknown sources..."
adb shell settings put secure install_non_market_apps 1
adb shell settings put global install_non_market_apps 1

# پیدا کردن فایل APK
APK_FILE=$(find . -name "*.apk" -type f | head -1)

if [ -z "$APK_FILE" ]; then
    echo "❌ No APK file found. Searching..."
    find . -name "*.apk" -type f
    exit 1
fi

echo "📦 Found APK: $APK_FILE"

# بررسی امضا
echo "🔏 Checking APK signature..."
apksigner verify --verbose "$APK_FILE" || echo "APK not signed, trying to install anyway..."

# نصب
echo "🚀 Installing APK..."
adb install -r "$APK_FILE"

if [ $? -eq 0 ]; then
    echo "🎉 Installation successful!"
    echo "📱 Launching app..."
    adb shell am start -n com.kohksh.app/.MainActivity
else
    echo "❌ Installation failed. Trying alternative method..."
    
    # روش جایگزین: push و install
    adb push "$APK_FILE" /data/local/tmp/
    adb shell pm install -r /data/local/tmp/$(basename "$APK_FILE")
fi
