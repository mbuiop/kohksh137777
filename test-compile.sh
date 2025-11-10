#!/bin/bash
echo "🎯 Direct Build - No Dependencies"

# پیدا کردن فایل‌های منبع اصلی
echo "🔍 Finding source files..."
CPP_FILES=$(find . -name "*.cpp" -type f | head -5)
C_FILES=$(find . -name "*.c" -type f | head -5)

echo "📁 Found CPP files: $CPP_FILES"
echo "📁 Found C files: $C_FILES"

if [ -n "$CPP_FILES" ] || [ -n "$C_FILES" ]; then
    echo "🔨 Building from project sources..."
    
    # کامپایل تمام فایل‌های پیدا شده
    g++ -o kohksh $CPP_FILES $C_FILES -std=c++17 -O2 -lstdc++ -lm -lpthread
    
    if [ -f "kohksh" ]; then
        echo "✅ Project build successful!"
        ./kohksh --version || ./kohksh -v || echo "✅ Binary created"
    else
        echo "❌ Project build failed, using minimal version"
        ./build-minimal.sh
    fi
else
    echo "📦 No source files found, using minimal version"
    ./build-minimal.sh
fi
