#!/bin/bash
echo "🎯 Creating SINGLE APK (no ZIP)..."

# ساخت فایل APK که مستقیم دانلود شود
./build-real-apk.sh

echo "📦 APK file is ready:"
ls -lh kohksh-real.apk

echo "✅ Now the APK can be downloaded directly without ZIP!"
