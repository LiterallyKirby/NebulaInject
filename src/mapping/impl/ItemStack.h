#pragma once
#include "jni.h"

class ItemStack {
private:
    JNIEnv* env;
    jobject itemStackObj;

public:
    ItemStack(JNIEnv* env, jobject obj) : env(env), itemStackObj(env->NewGlobalRef(obj)) {}

    ~ItemStack() {
        if (itemStackObj && env)
            env->DeleteGlobalRef(itemStackObj);
    }

    jobject GetItem(JNIEnv* env);
    bool IsWeapon(JNIEnv* env);
    bool IsBlock(JNIEnv* env);
    bool IsEnderPearl(JNIEnv* env);
    bool Is(const char* clazz, JNIEnv* env);
    int GetMetadata(JNIEnv* env);
};
