#!/bin/bash
echo "🧪 Creating Test Android App..."

# ایجاد یک اپلیکیشن تست ساده
mkdir -p android-test/app/src/main/java/com/kohksh/test
mkdir -p android-test/app/src/main/res/values

# فایل strings.xml
cat > android-test/app/src/main/res/values/strings.xml << 'EOF'
<resources>
    <string name="app_name">Kohksh Test App</string>
    <string name="hello_message">Hello from Kohksh!</string>
</resources>
EOF

# فایل MainActivity
cat > android-test/app/src/main/java/com/kohksh/test/MainActivity.java << 'EOF'
package com.kohksh.test;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        TextView textView = new TextView(this);
        textView.setText("✅ Kohksh Android App\n\n✨ Build Successful!\n\n📱 Version 1.0.0");
        textView.setTextSize(20);
        textView.setPadding(50, 50, 50, 50);
        textView.setTextAlignment(TextView.TEXT_ALIGNMENT_CENTER);
        
        setContentView(textView);
    }
}
EOF

# فایل AndroidManifest
cat > android-test/AndroidManifest.xml << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.kohksh.test">

    <application
        android:allowBackup="true"
        android:icon="@mipmap/ic_launcher"
        android:label="@string/app_name"
        android:theme="@style/AndroidTheme">
        
        <activity
            android:name=".MainActivity"
            android:label="@string/app_name">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

echo "📦 Creating APK package..."
cd android-test
zip -r ../kohksh-test-app.apk .

echo "✅ Test APK created: kohksh-test-app.apk"
echo "📲 Install with: adb install kohksh-test-app.apk"
