#!/bin/bash
echo "📱 Building Kohksh for Android..."

# تنظیم مسیرها
export NDK_PROJECT_PATH=$(pwd)
export APP_BUILD_SCRIPT=$(pwd)/android/Android.mk

echo "🔧 Setting up Android NDK build..."

# ایجاد دایرکتوری‌های ضروری
mkdir -p libs
mkdir -p obj

# کپی فایل main.cpp به دایرکتوری android اگر وجود ندارد
if [ ! -f "android/main.cpp" ]; then
    echo "📄 Creating main.cpp in android directory..."
    cp android/main.cpp android/main.cpp.backup 2>/dev/null || true
fi

echo "🔨 Starting NDK build..."
cd android

# ساخت با NDK
$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=.. \
    APP_BUILD_SCRIPT=./Android.mk \
    NDK_APPLICATION_MK=./Application.mk \
    APP_ABI=arm64-v8a,armeabi-v7a \
    V=1

# بررسی نتیجه
if [ -f "../libs/arm64-v8a/libkohksh_android.so" ]; then
    echo "✅ Android library built successfully!"
    echo "📦 Built files:"
    find ../libs -name "*.so" | head -10
else
    echo "❌ Build failed"
    echo "📁 Checking what files exist:"
    find .. -name "*.cpp" -o -name "*.c" | head -10
    echo "📁 libs directory:"
    ls -la ../libs/ 2>/dev/null || echo "libs directory not found"
fi
