#!/bin/bash
echo "📲 Direct Android Installation"

# بررسی اتصال دستگاه
echo "🔍 Checking Android device..."
adb devices

# فعال کردن نصب از منابع ناشناس
echo "🔓 Enabling unknown sources..."
adb shell settings put secure install_non_market_apps 1

# پیدا کردن فایل APK
if [ -f "kohksh-android.apk" ]; then
    echo "📦 Installing kohksh-android.apk..."
    adb install -r kohksh-android.apk
else
    echo "❌ APK not found, creating simple one..."
    
    # ایجاد یک APK ساده
    mkdir -p simple-apk
    cat > simple-apk/AndroidManifest.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.kohksh.simple">

    <application android:label="Kohksh Simple">
        <activity android:name=".MainActivity">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF
    cd simple-apk
    zip -r ../kohksh-simple.apk .
    cd ..
    
    echo "📦 Installing simple APK..."
    adb install -r kohksh-simple.apk
fi

if [ $? -eq 0 ]; then
    echo "🎉 Installation successful!"
    echo "🚀 Launching app..."
    adb shell am start -n com.kohksh.app/.MainActivity
else
    echo "❌ Installation failed"
    echo "💡 Try enabling USB debugging and Unknown sources in Android settings"
fi
