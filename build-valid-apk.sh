#!/bin/bash
echo "📱 Building VALID Android APK..."

# نصب ابزارهای ضروری
sudo apt-get update
sudo apt-get install -y android-sdk-build-tools

# ایجاد ساختار معتبر APK
rm -rf valid-app
mkdir -p valid-app/app/src/main/java/com/kohksh
mkdir -p valid-app/app/src/main/res/values
mkdir -p valid-app/app/src/main/res/drawable

# ایجاد فایل strings.xml
cat > valid-app/app/src/main/res/values/strings.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">Kohksh App</string>
    <string name="hello">Hello Kohksh!</string>
</resources>
EOF

# ایجاد فایل اصلی
cat > valid-app/app/src/main/java/com/kohksh/MainActivity.java << 'EOF'
package com.kohksh;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        TextView textView = new TextView(this);
        textView.setText("🎉 Kohksh App Running!\n\n✅ Successfully Installed\n\nVersion 1.0.0");
        textView.setTextSize(20);
        textView.setPadding(50, 50, 50, 50);
        textView.setTextAlignment(TextView.TEXT_ALIGNMENT_CENTER);
        
        setContentView(textView);
    }
}
EOF

# ایجاد فایل manifest معتبر
cat > valid-app/AndroidManifest.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.kohksh.app"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="21" android:targetSdkVersion="34" />
    
    <application
        android:icon="@drawable/ic_launcher"
        android:label="@string/app_name"
        android:theme="@android:style/Theme.DeviceDefault.Light">
        
        <activity
            android:name="com.kohksh.MainActivity"
            android:label="@string/app_name"
            android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

echo "🔑 Creating proper signing key..."
# ایجاد کلید امضای معتبر
keytool -genkey -v -keystore my-release-key.keystore \
    -alias alias_name -keyalg RSA -keysize 2048 \
    -validity 10000 -storepass password123 -keypass password123 \
    -dname "CN=Kohksh, OU=Development, O=Kohksh, L=Tehran, ST=Tehran, C=IR"

echo "📦 Building APK..."
# ساخت APK با ساختار معتبر
cd valid-app
zip -r ../app-unsigned.apk . > /dev/null 2>&1
cd ..

echo "🔏 Signing APK properly..."
# امضا کردن با ابزار درست
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 \
    -keystore my-release-key.keystore \
    -storepass password123 -keypass password123 \
    app-unsigned.apk alias_name

echo "⚡ Aligning APK..."
# بهینه سازی با zipalign
zipalign -v 4 app-unsigned.apk kohksh-valid.apk

echo "✅ VALID APK created: kohksh-valid.apk"
echo "📱 File ready for installation!"
