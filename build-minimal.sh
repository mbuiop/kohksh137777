#!/bin/bash
echo "🏗️ Building Minimal Kohksh..."

# ایجاد یک مینیمال ترین برنامه ممکن
cat > minimal.cpp << 'EOF'
#include <iostream>

int main() {
    std::cout << "=================================" << std::endl;
    std::cout << "🚀 KOHKSH APPLICATION RUNNING!" << std::endl;
    std::cout << "✅ Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "✅ Version: 1.0.0" << std::endl;
    std::cout << "✅ Platform: Linux" << std::endl;
    std::cout << "=================================" << std::endl;
    return 0;
}
EOF

# کامپایل
g++ -o kohksh minimal.cpp -std=c++17 -O2

if [ -f "kohksh" ]; then
    echo "✅ Minimal build successful!"
    echo "📦 Binary info:"
    file kohksh
    echo "🎯 Running test:"
    ./kohksh
else
    echo "❌ Minimal build failed"
    exit 1
fi
