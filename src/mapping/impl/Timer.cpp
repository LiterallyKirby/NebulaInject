#include "Timer.h"
#include <jni.h>
#include "../Mapping.h"

float Timer::GetRenderPartialTicks(JNIEnv* env) {
    if (!env) return 0.0f;

    // If your pattern is that 'this' is really a jobject, keep using reinterpret_cast.
    // If your Timer wrapper stores a jobject field, use that instead.
    jobject obj = reinterpret_cast<jobject>(this);
    if (!obj) return 0.0f;

    jclass cls = env->GetObjectClass(obj);
    if (!cls) return 0.0f;

    // !!! IMPORTANT: keep the std::string alive while we call GetFieldID
    std::string fieldNameStr = Mapping::Get("renderPartialTicks");
    const char* fieldName = fieldNameStr.c_str();

    jfieldID fid = env->GetFieldID(cls, fieldName, "F");
    if (!fid) {
        env->DeleteLocalRef(cls);
        return 0.0f;
    }

    jfloat value = env->GetFloatField(obj, fid);
    env->DeleteLocalRef(cls);
    return static_cast<float>(value);
}

float Timer::GetTimerSpeed(JNIEnv* env) {
    if (!env) return 0.0f;

    jobject obj = reinterpret_cast<jobject>(this);
    if (!obj) return 0.0f;

    jclass cls = env->GetObjectClass(obj);
    if (!cls) return 0.0f;

    std::string fieldNameStr = Mapping::Get("timerSpeed");
    const char* fieldName = fieldNameStr.c_str();

    jfieldID fid = env->GetFieldID(cls, fieldName, "F");
    if (!fid) {
        env->DeleteLocalRef(cls);
        return 0.0f;
    }

    jfloat value = env->GetFloatField(obj, fid);
    env->DeleteLocalRef(cls);
    return static_cast<float>(value);
}

void Timer::SetTimerSpeed(float v, JNIEnv* env) {
    if (!env) return;

    jobject obj = reinterpret_cast<jobject>(this);
    if (!obj) return;

    jclass cls = env->GetObjectClass(obj);
    if (!cls) return;

    std::string fieldNameStr = Mapping::Get("timerSpeed");
    const char* fieldName = fieldNameStr.c_str();

    jfieldID fid = env->GetFieldID(cls, fieldName, "F");
    if (!fid) {
        env->DeleteLocalRef(cls);
        return;
    }

    env->SetFloatField(obj, fid, static_cast<jfloat>(v));
    env->DeleteLocalRef(cls);
}
