LOCAL_PATH := $(call my-dir)

# ابتدا SDL را build می‌کنیم
include $(CLEAR_VARS)

LOCAL_MODULE := SDL3
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := \
    SDL_android.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES

include $(BUILD_STATIC_LIBRARY)

# حالا پروژه اصلی را build می‌کنیم
include $(CLEAR_VARS)

LOCAL_MODULE := kohksh_android

# فایل‌های منبع پروژه - همه فایل‌های موجود
LOCAL_SRC_FILES := \
    main.cpp

# اگر فایل‌های دیگری وجود دارد، اضافه کنید
LOCAL_SRC_FILES += $(wildcard ../*.cpp)
LOCAL_SRC_FILES += $(wildcard ../*.c)
LOCAL_SRC_FILES += $(wildcard ../src/*.cpp)
LOCAL_SRC_FILES += $(wildcard ../src/*.c)

# شامل کردن SDL
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES := SDL3
LOCAL_LDLIBS := -llog -landroid -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -DANDROID

include $(BUILD_SHARED_LIBRARY)
