#!/bin/bash
echo "🔏 Signing APK for installation..."

APK_FILE="kohksh-app-unsigned.apk"
SIGNED_APK="kohksh-signed.apk"

# پیدا کردن فایل APK
if [ ! -f "$APK_FILE" ]; then
    APK_FILE=$(find . -name "*.apk" -type f | head -1)
    if [ -z "$APK_FILE" ]; then
        echo "❌ No APK file found"
        exit 1
    fi
fi

echo "📦 Found APK: $APK_FILE"

# ایجاد کلید امضا
if [ ! -f "debug.keystore" ]; then
    echo "🔑 Creating debug keystore..."
    keytool -genkey -v -keystore debug.keystore \
        -alias androiddebugkey -keyalg RSA \
        -keysize 2048 -validity 10000 \
        -storepass android -keypass android \
        -dname "CN=Android Debug,O=Android,C=US"
fi

# امضا کردن APK
echo "📝 Signing APK..."
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 \
    -keystore debug.keystore \
    -storepass android -keypass android \
    "$APK_FILE" androiddebugkey

# بهینه سازی
echo "⚡ Optimizing APK..."
zipalign -v 4 "$APK_FILE" "$SIGNED_APK"

echo "✅ Signed APK created: $SIGNED_APK"
echo "📲 Install with: adb install $SIGNED_APK"
