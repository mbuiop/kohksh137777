#!/bin/bash
echo "🔍 Debug Build Process"

echo "📁 Current directory:"
pwd

echo "📁 Files in root:"
ls -la

echo "📁 GitHub workspace:"
ls -la $GITHUB_WORKSPACE

echo "🔧 Environment:"
echo "GITHUB_WORKSPACE: $GITHUB_WORKSPACE"
echo "PWD: $PWD"

# تست ایجاد فایل
echo "🧪 Creating test file..."
echo "test content" > test-file.txt
ls -la test-file.txt

# تست ایجاد APK
mkdir -p debug-apk
echo "debug apk" > debug-apk/test.txt
cd debug-apk
zip ../debug-test.apk test.txt
cd ..

echo "📦 Test APK created:"
ls -la *.apk

echo "✅ Debug completed"
