#!/bin/bash
echo "📱 Building Kohksh for Android..."

# ایجاد دایرکتوری‌های لازم
mkdir -p android-build
mkdir -p android/libs

echo "🔧 Setting up Android build..."
cd android-build

# استفاده از NDK
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=.. NDK_APPLICATION_MK=../android/Application.mk

if [ -f "../libs/arm64-v8a/libkohksh_android.so" ]; then
    echo "✅ Android build successful!"
    ls -la ../libs/
else
    echo "❌ Android build failed"
    echo "Trying alternative build method..."
    cd ..
    $ANDROID_NDK_HOME/ndk-build -C android
fi
