LOCAL_PATH := $(call my-dir)

# ابتدا SDL را build می‌کنیم
include $(CLEAR_VARS)

LOCAL_MODULE := SDL3
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := \
    SDL_android.c \
    SDL_androidgl.c \
    SDL_androidmessagebox.c \
    SDL_androidwindow.c \
    SDL_androidvulkan.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -landroid

include $(BUILD_STATIC_LIBRARY)

# حالا پروژه اصلی را build می‌کنیم
include $(CLEAR_VARS)

LOCAL_MODULE := kohksh_android

# فایل‌های منبع پروژه شما
LOCAL_SRC_FILES := \
    ../main.cpp \
    ../kohksh.cpp \
    $(wildcard ../src/*.cpp) \
    $(wildcard ../src/*.c)

# شامل کردن SDL
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../src

LOCAL_STATIC_LIBRARIES := SDL3
LOCAL_LDLIBS := -llog -landroid -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -DANDROID -DSDL_ANDROID

include $(BUILD_SHARED_LIBRARY)
