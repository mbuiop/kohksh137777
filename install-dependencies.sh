#!/bin/bash
echo "📦 Installing Kohksh dependencies..."

# به روز رسانی سیستم
sudo apt-get update

# نصب وابستگی‌های اصلی
sudo apt-get install -y \
  build-essential \
  cmake \
  pkg-config \
  git

# کتابخانه‌های SDL2
sudo apt-get install -y \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-ttf-dev \
  libsdl2-mixer-dev \
  libsdl2-net-dev

# کتابخانه‌های OpenGL
sudo apt-get install -y \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libglew-dev \
  libglfw3-dev

# کتابخانه‌های صوتی
sudo apt-get install -y \
  libopenal-dev \
  libalut-dev \
  libvorbis-dev \
  libflac-dev \
  libmpg123-dev

# کتابخانه‌های چندرسانه‌ای
sudo apt-get install -y \
  libavcodec-dev \
  libavformat-dev \
  libavutil-dev \
  libswscale-dev \
  libavdevice-dev

# کتابخانه‌های X11
sudo apt-get install -y \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libxcursor-dev \
  libxinerama-dev \
  libxcomposite-dev \
  libxdamage-dev

echo "✅ All dependencies installed!"
