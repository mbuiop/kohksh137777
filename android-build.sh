#!/bin/bash
echo "📱 Building Kohksh for Android..."

# تنظیم مسیرها
export NDK_PROJECT_PATH=$(pwd)
export APP_BUILD_SCRIPT=$(pwd)/android/Android.mk

echo "🔧 Setting up Android NDK build..."

# ایجاد دایرکتوری‌های ضروری
mkdir -p libs
mkdir -p obj

# ساخت کتابخانه با NDK
cd android
$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=.. \
    APP_BUILD_SCRIPT=./Android.mk \
    NDK_APPLICATION_MK=./Application.mk \
    APP_ABI=arm64-v8a,armeabi-v7a

# بررسی نتیجه
if [ -f "../libs/arm64-v8a/libkohksh_android.so" ]; then
    echo "✅ Android library built successfully!"
    
    # ایجاد فایل AAR ساده
    echo "📦 Creating Android AAR package..."
    mkdir -p ../kohksh-aar/jni
    cp -r ../libs/* ../kohksh-aar/jni/
    
    # ایجاد فایل APK تستی
    echo "📱 Creating test APK..."
    mkdir -p ../test-apk/lib
    cp -r ../libs/* ../test-apk/lib/
    
    echo "🎉 Build completed! Files ready in libs/ directory"
else
    echo "❌ Build failed - no library files found"
    exit 1
fi
