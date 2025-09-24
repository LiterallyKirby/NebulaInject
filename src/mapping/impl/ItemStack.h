#pragma once
#include <jni.h>

class ItemStack {
public:
    ItemStack(JNIEnv* env, jobject obj);
    ~ItemStack();

    // existing methods...
    jobject GetItem(JNIEnv* env);
    bool IsWeapon();
    bool IsBlock(JNIEnv* env);
    bool IsEnderPearl(JNIEnv* env);
    bool Is(const char* clazz, JNIEnv* env);
    int GetMetadata(JNIEnv* env);
JNIEnv* GetEnv(); 
private:
    JNIEnv* env = nullptr;
    jobject itemStackObj = nullptr; // global ref managed in cpp
    JavaVM* jvm = nullptr;
};
