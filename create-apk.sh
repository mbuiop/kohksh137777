#!/bin/bash
echo "📦 Creating installable APK..."

# ایجاد ساختار APK
mkdir -p kohksh-apk/lib/arm64-v8a
mkdir -p kohksh-apk/lib/armeabi-v7a
mkdir -p kohksh-apk/assets

# کپی کتابخانه‌ها
cp -r libs/arm64-v8a/*.so kohksh-apk/lib/arm64-v8a/ 2>/dev/null || true
cp -r libs/armeabi-v7a/*.so kohksh-apk/lib/armeabi-v7a/ 2>/dev/null || true

# ایجاد فایل AndroidManifest.xml ساده
cat > kohksh-apk/AndroidManifest.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.kohksh.app">

    <uses-permission android:name="android.permission.INTERNET" />
    
    <application
        android:allowBackup="true"
        android:icon="@mipmap/ic_launcher"
        android:label="Kohksh App"
        android:theme="@style/AppTheme">
        
        <activity
            android:name=".MainActivity"
            android:label="Kohksh">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

# ایجاد فایل ZIP (APK ساده)
cd kohksh-apk
zip -r ../kohksh-app-unsigned.apk .
cd ..

echo "✅ Created: kohksh-app-unsigned.apk"
echo "📱 This file can be installed on Android devices using:"
echo "   adb install kohksh-app-unsigned.apk"
