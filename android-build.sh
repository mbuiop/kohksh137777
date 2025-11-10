#!/bin/bash
echo "📱 Building Kohksh for Android..."

# تنظیم مسیرهای ضروری
export NDK_PROJECT_PATH=$(pwd)
export APP_BUILD_SCRIPT=$(pwd)/android/Android.mk
export NDK_APPLICATION_MK=$(pwd)/android/Application.mk

echo "📁 NDK_PROJECT_PATH: $NDK_PROJECT_PATH"
echo "📁 APP_BUILD_SCRIPT: $APP_BUILD_SCRIPT"
echo "📁 NDK_APPLICATION_MK: $NDK_APPLICATION_MK"

# بررسی وجود NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "❌ ANDROID_NDK_HOME not set"
    exit 1
else
    echo "✅ ANDROID_NDK_HOME: $ANDROID_NDK_HOME"
fi

# ایجاد دایرکتوری خروجی
mkdir -p android/libs
mkdir -p android/obj

echo "🔨 Starting NDK build..."
cd android

# اجرای ndk-build با مسیرهای صحیح
$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=.. \
    NDK_APPLICATION_MK=./Application.mk \
    APP_BUILD_SCRIPT=./Android.mk

# بررسی نتیجه ساخت
if [ -f "../libs/arm64-v8a/libkohksh_android.so" ]; then
    echo "✅ Android build successful!"
    echo "📦 Built libraries:"
    find ../libs -name "*.so" | head -10
else
    echo "❌ Android build failed - no libraries found"
    echo "📁 Checking build directory:"
    ls -la ../libs/ || echo "libs directory not found"
    ls -la ../obj/ || echo "obj directory not found"
fi
