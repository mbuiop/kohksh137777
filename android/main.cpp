#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "Kohksh"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_kohksh_MainActivity_getMessage(JNIEnv *env, jobject thiz) {
    LOGI("Kohksh Android Library Loaded!");
    return env->NewStringUTF("Hello from Kohksh Android!");
}

}
