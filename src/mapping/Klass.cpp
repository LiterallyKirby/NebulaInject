#include "Klass.h"
#include "Field.h"
#include "Method.h"
#include "jni.h"
#include <string>
#include <iostream>

std::string Klass::GetName(JNIEnv* env)
{
    if (!env || !clazz) {
        std::cerr << "Error: JNIEnv or class is null in GetName" << std::endl;
        return "";
    }
    
    // Check for pending exceptions
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    
    const auto cls = env->FindClass("java/lang/Class");
    if (!cls) {
        std::cerr << "Error: Could not find java/lang/Class" << std::endl;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        return "";
    }
    
    const auto mid_getName = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
    if (!mid_getName) {
        std::cerr << "Error: Could not find getName method" << std::endl;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(cls);
        return "";
    }
    
    // Use clazz instead of casting this
    jstring jname = (jstring)env->CallObjectMethod(clazz, mid_getName);
    if (!jname || env->ExceptionCheck()) {
        std::cerr << "Error: CallObjectMethod failed" << std::endl;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(cls);
        return "";
    }
    
    const char* utfName = env->GetStringUTFChars(jname, nullptr);
    std::string nameStr = utfName ? utfName : "";
    if (utfName)
        env->ReleaseStringUTFChars(jname, utfName);
    
    env->DeleteLocalRef(jname);
    env->DeleteLocalRef(cls);
    return nameStr;
}

Field* Klass::GetField(JNIEnv* env, const char* name, const char* sig, bool staticField)
{
    // Enhanced null checks
    if (!env) {
        std::cerr << "Error: JNIEnv is null in GetField" << std::endl;
        return nullptr;
    }
    
    if (!clazz) {
        std::cerr << "Error: Class is null in GetField" << std::endl;
        return nullptr;
    }
    
    if (!name || !sig) {
        std::cerr << "Error: Field name or signature is null" << std::endl;
        std::cerr << "Name: " << (name ? name : "NULL") << std::endl;
        std::cerr << "Signature: " << (sig ? sig : "NULL") << std::endl;
        return nullptr;
    }
    
    
    
    // Check for pending exceptions before making the call
    if (env->ExceptionCheck()) {
        std::cerr << "Error: JNI exception pending before GetFieldID" << std::endl;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    
    jfieldID fid = staticField
        ? env->GetStaticFieldID(clazz, name, sig)  // Use clazz, not cast this
        : env->GetFieldID(clazz, name, sig);       // Use clazz, not cast this
    
    // Check for exceptions after the call
    if (env->ExceptionCheck()) {
        std::cerr << "Error: JNI exception occurred in GetFieldID for field: " << name << std::endl;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    
    if (!fid) {
        std::cerr << "Error: Failed to get field ID for: " << name << std::endl;
        return nullptr;
    }
    
    // Create proper Field object - check Field class constructor
    // If Field doesn't have a constructor taking jfieldID, we need to use the original approach
    // but with proper safety checks
    if (!fid) {
        return nullptr;
    }
    return reinterpret_cast<Field*>(fid);
}

Method* Klass::GetMethod(JNIEnv* env, const char* name, const char* sig, bool staticMethod)
{
    // Enhanced null checks
    if (!env) {
        std::cerr << "Error: JNIEnv is null in GetMethod" << std::endl;
        return nullptr;
    }
    
    if (!clazz) {
        std::cerr << "Error: Class is null in GetMethod" << std::endl;
        return nullptr;
    }
    
    if (!name || !sig) {
        std::cerr << "Error: Method name or signature is null" << std::endl;
        return nullptr;
    }
    
    // Check for pending exceptions
    if (env->ExceptionCheck()) {
        std::cerr << "Error: JNI exception pending before GetMethodID" << std::endl;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    
    jmethodID mid = staticMethod
        ? env->GetStaticMethodID(clazz, name, sig)  // Use clazz, not cast this
        : env->GetMethodID(clazz, name, sig);       // Use clazz, not cast this
    
    // Check for exceptions after the call
    if (env->ExceptionCheck()) {
        std::cerr << "Error: JNI exception occurred in GetMethodID for method: " << name << std::endl;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    
    if (!mid) {
        std::cerr << "Error: Failed to get method ID for: " << name << std::endl;
        return nullptr;
    }
    
    // Create proper Method object - check Method class constructor
    // If Method doesn't have a constructor taking jmethodID, we need to use the original approach
    // but with proper safety checks
    if (!mid) {
        return nullptr;
    }
    return reinterpret_cast<Method*>(mid);
}
