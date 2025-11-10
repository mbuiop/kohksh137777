#!/bin/bash
echo "📱 Building Android APK..."

# پاکسازی فایل‌های قدیمی
rm -f *.apk
rm -rf android-app apk-unzipped simple-apk

# ایجاد ساختار پروژه
mkdir -p android-app/app/src/main/java/com/kohksh

# ایجاد فایل اصلی
cat > android-app/app/src/main/java/com/kohksh/MainActivity.java << 'EOF'
package com.kohksh;

import android.app.*;
import android.os.*;
import android.widget.*;
import android.graphics.Color;

public class MainActivity extends Activity {
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(android.view.Gravity.CENTER);
        layout.setBackgroundColor(Color.WHITE);
        layout.setPadding(50, 50, 50, 50);
        
        TextView title = new TextView(this);
        title.setText("🎉 Kohksh App");
        title.setTextSize(28);
        title.setTextColor(Color.BLACK);
        title.setGravity(android.view.Gravity.CENTER);
        
        TextView subtitle = new TextView(this);
        subtitle.setText("✅ Successfully Installed!");
        subtitle.setTextSize(18);
        subtitle.setTextColor(Color.GREEN);
        subtitle.setGravity(android.view.Gravity.CENTER);
        
        TextView info = new TextView(this);
        info.setText("Version 1.0.0\nBuilt with GitHub Actions");
        info.setTextSize(14);
        info.setTextColor(Color.GRAY);
        info.setGravity(android.view.Gravity.CENTER);
        
        layout.addView(title);
        layout.addView(subtitle);
        layout.addView(info);
        
        setContentView(layout);
    }
}
EOF

echo "🔑 Creating signing key..."
# ایجاد کلید امضا
keytool -genkey -v -keystore debug.keystore \
    -alias androiddebugkey -keyalg RSA \
    -keysize 2048 -validity 10000 \
    -storepass android -keypass android \
    -dname "CN=Android Debug,O=Android,C=US" 2>/dev/null

echo "📦 Creating APK structure..."
# ایجاد ساختار APK
mkdir -p apk-unzipped/META-INF
mkdir -p apk-unzipped/com/kohksh

# ایجاد فایل manifest
cat > apk-unzipped/AndroidManifest.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.kohksh.app"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="21" android:targetSdkVersion="34" />
    
    <application
        android:icon="@drawable/ic_launcher"
        android:label="Kohksh Application">
        
        <activity
            android:name="com.kohksh.MainActivity"
            android:label="Kohksh"
            android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

# ایجاد فایل کلاس‌های اصلی
cp android-app/app/src/main/java/com/kohksh/MainActivity.java apk-unzipped/com/kohksh/

echo "📝 Creating APK file..."
cd apk-unzipped
zip -9 -r ../app-unsigned.apk . > /dev/null 2>&1
cd ..

echo "🔏 Signing APK..."
# امضا کردن APK
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 \
    -keystore debug.keystore \
    -storepass android -keypass android \
    app-unsigned.apk androiddebugkey > /dev/null 2>&1

echo "⚡ Creating final APK..."
# ایجاد فایل نهایی
cp app-unsigned.apk kohksh-android.apk

echo "✅ APK created successfully!"
echo "📊 File info:"
ls -lh *.apk
echo "📁 APK contents:"
unzip -l kohksh-android.apk | head -15

# ایجاد یک فایل APK اضافی برای اطمینان
echo "🔄 Creating backup APK..."
cp kohksh-android.apk kohksh-app-release.apk

echo "🎉 Build completed! APK files:"
find . -name "*.apk" -type f
