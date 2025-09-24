#pragma once
#include "jni.h"
#include <string>
class Field;
class Method;

class Klass {
    jclass clazz;
public:
    Klass(jclass c) : clazz(c) {} // <- this fixes the "too many initializers" error

   std::string GetName(JNIEnv* env);
    Field* GetField(JNIEnv* env, const char* name, const char* sig, bool staticField = false);
    Method* GetMethod(JNIEnv* env, const char* name, const char* sig, bool staticMethod = false);

    static Klass* Find(JNIEnv* env, const char* name) {
        jclass clazz = env->FindClass(name);
        return clazz ? new Klass(clazz) : nullptr;
    }
};

