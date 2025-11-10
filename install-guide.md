# 📱 راهنمای نصب Kohksh روی اندروید

## روش ۱: فعال کردن منابع ناشناس
1. **Settings** → **Security** → **Unknown Sources**
2. فعال کردن **Allow installation from unknown sources**

## روش ۲: نصب با ADB
```bash
# دانلود APK
adb install kohksh-working.apk

# اگر خطا داد:
adb install -r -g kohksh-working.apk
