#!/bin/bash
echo "🔧 Alternative NDK build method..."

# روش مستقیم با ndk-build
$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=. \
    APP_BUILD_SCRIPT=./Android.mk \
    NDK_APPLICATION_MK=./Application.mk \
    APP_ABI=arm64-v8a,armeabi-v7a \
    APP_PLATFORM=android-21

echo "Build completed with status: $?"
