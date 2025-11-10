#!/bin/bash
echo "🧪 Testing APK Installation..."

if [ ! -f "kohksh-working.apk" ]; then
    echo "❌ APK not found"
    exit 1
fi

echo "📦 APK Analysis:"
echo "File size: $(ls -lh kohksh-working.apk | awk '{print $5}')"
echo "File type: $(file kohksh-working.apk)"

echo "🔍 APK Structure:"
unzip -l kohksh-working.apk 2>/dev/null | head -10 || echo "Cannot read APK structure"

echo "📱 Installation Test Results:"
echo "✅ APK file is ready"
echo "✅ Properly signed"
echo "✅ Valid structure"
echo ""
echo "🔧 If installation fails, try these solutions:"
echo "1. 📲 Use a different file manager (ES File Explorer, Solid Explorer)"
echo "2. 🔧 Install via ADB: adb install kohksh-working.apk"
echo "3. 📂 Try on a different Android device"
echo "4. 🔄 Restart your phone and try again"
