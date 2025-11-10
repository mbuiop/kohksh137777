#!/bin/bash
echo "🏗️ Building Kohksh for Linux..."

# بررسی وجود فایل‌های سازنده
if [ -f "autogen.sh" ]; then
    echo "📦 Running autogen.sh..."
    ./autogen.sh
fi

if [ -f "configure" ]; then
    echo "⚙️ Running configure..."
    ./configure --prefix=/usr
fi

if [ -f "Makefile" ]; then
    echo "🔨 Compiling..."
    make -j$(nproc)
    
    if [ -f "kohksh" ]; then
        echo "✅ Build successful! Output: ./kohksh"
        ./kohksh --version || echo "✅ Binary created successfully"
    else
        echo "🔍 Looking for output binary..."
        find . -type f -executable -not -path "./.*" | head -10
    fi
else
    echo "❌ No Makefile found"
    echo "📁 Current directory:"
    ls -la
fi
