// ItemStack.cpp
#include "ItemStack.h"

#include <iostream>

#include "../Field.h"
#include "../Klass.h"
#include "../Mapping.h"
#include "../Method.h"

// ItemStack.cpp (constructor + destructor)

#include <jni.h>

#include <iostream>

// Make sure ItemStack class has a JavaVM* jvm member (in ItemStack.h)

ItemStack::ItemStack(JNIEnv* env_, jobject obj)
    : env(env_), itemStackObj(nullptr), jvm(nullptr) {
    if (!env_ || !obj) return;

    if (env_->GetJavaVM(&jvm) != JNI_OK) {
        std::cerr << "[ItemStack] Warning: GetJavaVM failed\n";
        jvm = nullptr;
    }

    itemStackObj = env_->NewGlobalRef(obj);
    if (!itemStackObj) {
        std::cerr << "[ItemStack] Failed to create global ref for ItemStack\n";
    }
}

JNIEnv* ItemStack::GetEnv() {
    if (!jvm) return nullptr;

    JNIEnv* e = nullptr;
    if (jvm->GetEnv((void**)&e, JNI_VERSION_1_6) != JNI_OK) {
        if (jvm->AttachCurrentThread((void**)&e, nullptr) != JNI_OK) {
            std::cerr << "[ItemStack] Failed to attach current thread\n";
            return nullptr;
        }
    }
    return e;
}
ItemStack::~ItemStack() {
    if (!itemStackObj) return;

    JNIEnv* env = GetEnv();
    if (env) {
        env->DeleteGlobalRef(itemStackObj);
    } else {
        std::cerr << "[ItemStack::~ItemStack] Could not obtain JNIEnv to "
                     "DeleteGlobalRef; leaking reference to avoid crash\n";
    }

    itemStackObj = nullptr;
}

// Return a local-ref jobject to the Java Item (not the ItemStack).
// The returned jobject is a local reference created by JNI CallObjectMethod and
// is valid in the current env; the caller may treat it as a local ref (we will
// delete it after use in IsWeapon).

jobject ItemStack::GetItem(JNIEnv* env) {
    if (!env || !itemStackObj) return nullptr;

    const std::string itemStackClassName =
        Mapping::Get("net/minecraft/item/ItemStack");
    Klass* itemStackKlass = Klass::Find(env, itemStackClassName.c_str());
    if (!itemStackKlass) {
        std::cerr << "[ItemStack::GetItem] Failed to find class: "
                  << itemStackClassName << "\n";
        return nullptr;
    }

    // Get method name and correct signature from mappings
    const std::string methodName = Mapping::Get("getItem");  // likely "b"
    const std::string methodSig =
        Mapping::Get("net/minecraft/item/Item", 3);  // "()L...;"

    auto getItemMethod =
        itemStackKlass->GetMethod(env, methodName.c_str(), methodSig.c_str());
    if (!getItemMethod) {
        std::cerr << "[ItemStack::GetItem] Failed to find method " << methodName
                  << " sig " << methodSig << "\n";
        return nullptr;
    }

    jobject itemObj = getItemMethod->CallObjectMethod(env, itemStackObj);
    return itemObj;
}

bool ItemStack::IsWeapon() {
    JNIEnv* env = GetEnv();
    if (!env || !itemStackObj) {
        std::cerr << "[ItemStack::IsWeapon] Invalid env or itemStackObj\n";
        return false;
    }

    jobject itemObj = GetItem(env);
    if (!itemObj) {
        std::cerr << "[ItemStack::IsWeapon] itemObj is null\n";
        return false;
    }

    // Get mapped class names (these should return the obfuscated names like "aay", "yl")
    std::string swordClassName = Mapping::Get("net/minecraft/item/ItemSword");
    std::string axeClassName = Mapping::Get("net/minecraft/item/ItemAxe");
    
    bool isSword = false;
    bool isAxe = false;

    // Try to find and check sword class safely
    if (!swordClassName.empty()) {
        try {
            // Use JNI FindClass with obfuscated name
            jclass swordClass = env->FindClass(swordClassName.c_str());
            if (swordClass && !env->ExceptionCheck()) {
                isSword = env->IsInstanceOf(itemObj, swordClass);
                env->DeleteLocalRef(swordClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    isSword = false;
                }
            } else {
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                }
                std::cout << "[ItemStack::IsWeapon] Could not find sword class: " << swordClassName << "\n";
            }
        } catch (...) {
            std::cerr << "[ItemStack::IsWeapon] Exception during sword class check\n";
            isSword = false;
        }
    }

    // Try to find and check axe class safely
    if (!axeClassName.empty()) {
        try {
            // Use JNI FindClass with obfuscated name
            jclass axeClass = env->FindClass(axeClassName.c_str());
            if (axeClass && !env->ExceptionCheck()) {
                isAxe = env->IsInstanceOf(itemObj, axeClass);
                env->DeleteLocalRef(axeClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    isAxe = false;
                }
            } else {
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                }
                std::cout << "[ItemStack::IsWeapon] Could not find axe class: " << axeClassName << "\n";
            }
        } catch (...) {
            std::cerr << "[ItemStack::IsWeapon] Exception during axe class check\n";
            isAxe = false;
        }
    }

    // For debugging, also get the actual class name
    jclass itemClass = env->GetObjectClass(itemObj);
    std::string actualClassName = "unknown";
    if (itemClass) {
        jclass classClass = env->FindClass("java/lang/Class");
        if (classClass) {
            jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
            if (getNameMethod) {
                jstring classNameStr = (jstring)env->CallObjectMethod(itemClass, getNameMethod);
                if (classNameStr) {
                    const char* classNameChars = env->GetStringUTFChars(classNameStr, nullptr);
                    if (classNameChars) {
                        actualClassName = classNameChars;
                        env->ReleaseStringUTFChars(classNameStr, classNameChars);
                    }
                    env->DeleteLocalRef(classNameStr);
                }
            }
            env->DeleteLocalRef(classClass);
        }
        env->DeleteLocalRef(itemClass);
    }

    std::cout << "[ItemStack::IsWeapon] Item class: " << actualClassName 
              << ", Looking for Sword: '" << swordClassName << "' (" << isSword << ")"
              << ", Axe: '" << axeClassName << "' (" << isAxe << ")" 
              << ", Result: " << (isSword || isAxe) << "\n";

    env->DeleteLocalRef(itemObj);
    return isSword || isAxe;
}

// Keep your other methods but ensure they operate on itemStackObj (global ref)
bool ItemStack::IsBlock(JNIEnv* env) {
    if (!env || !itemStackObj) {
        std::cerr << "[ItemStack::IsBlock] Invalid env or itemStackObj\n";
        return false;
    }

    jobject itemObj = GetItem(env);
    if (!itemObj) {
        std::cerr << "[ItemStack::IsBlock] itemObj is null\n";
        return false;
    }

    // Get mapped class name (should return obfuscated name like "yo")
    std::string blockClassName = Mapping::Get("net/minecraft/item/ItemBlock");
    
    bool isBlock = false;

    // Try to find and check block class safely
    if (!blockClassName.empty()) {
        try {
            // Use JNI FindClass with obfuscated name
            jclass blockClass = env->FindClass(blockClassName.c_str());
            if (blockClass && !env->ExceptionCheck()) {
                isBlock = env->IsInstanceOf(itemObj, blockClass);
                env->DeleteLocalRef(blockClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    isBlock = false;
                }
            } else {
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                }
                std::cout << "[ItemStack::IsBlock] Could not find block class: " << blockClassName << "\n";
            }
        } catch (...) {
            std::cerr << "[ItemStack::IsBlock] Exception during block class check\n";
            isBlock = false;
        }
    }

    // For debugging, also get the actual class name
    jclass itemClass = env->GetObjectClass(itemObj);
    std::string actualClassName = "unknown";
    if (itemClass) {
        jclass classClass = env->FindClass("java/lang/Class");
        if (classClass) {
            jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
            if (getNameMethod) {
                jstring classNameStr = (jstring)env->CallObjectMethod(itemClass, getNameMethod);
                if (classNameStr) {
                    const char* classNameChars = env->GetStringUTFChars(classNameStr, nullptr);
                    if (classNameChars) {
                        actualClassName = classNameChars;
                        env->ReleaseStringUTFChars(classNameStr, classNameChars);
                    }
                    env->DeleteLocalRef(classNameStr);
                }
            }
            env->DeleteLocalRef(classClass);
        }
        env->DeleteLocalRef(itemClass);
    }

    std::cout << "[ItemStack::IsBlock] Item class: " << actualClassName 
              << ", Looking for Block: '" << blockClassName << "' (" << isBlock << ")"
              << ", Result: " << isBlock << "\n";

    env->DeleteLocalRef(itemObj);
    return isBlock;
}

bool ItemStack::IsEnderPearl(JNIEnv* env) {
    if (!env || !itemStackObj) {
        std::cerr << "[ItemStack::IsEnderPearl] Invalid env or itemStackObj\n";
        return false;
    }

    jobject itemObj = GetItem(env);
    if (!itemObj) {
        std::cerr << "[ItemStack::IsEnderPearl] itemObj is null\n";
        return false;
    }

    // Get mapped class name (should return obfuscated name like "zk")
    std::string pearlClassName = Mapping::Get("net/minecraft/item/ItemEnderPearl");
    
    bool isPearl = false;

    // Try to find and check ender pearl class safely
    if (!pearlClassName.empty()) {
        try {
            // Use JNI FindClass with obfuscated name
            jclass pearlClass = env->FindClass(pearlClassName.c_str());
            if (pearlClass && !env->ExceptionCheck()) {
                isPearl = env->IsInstanceOf(itemObj, pearlClass);
                env->DeleteLocalRef(pearlClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    isPearl = false;
                }
            } else {
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                }
                std::cout << "[ItemStack::IsEnderPearl] Could not find pearl class: " << pearlClassName << "\n";
            }
        } catch (...) {
            std::cerr << "[ItemStack::IsEnderPearl] Exception during pearl class check\n";
            isPearl = false;
        }
    }

    env->DeleteLocalRef(itemObj);
    return isPearl;
}

bool ItemStack::Is(const char* clazz, JNIEnv* env) {
    if (!env || !itemStackObj || !clazz) return false;
    jobject itemObj = GetItem(env);
    if (!itemObj) return false;
    Klass* k = Klass::Find(env, clazz);
    bool res = k && env->IsInstanceOf(itemObj, (jclass)k);
    env->DeleteLocalRef(itemObj);
    return res;
}

int ItemStack::GetMetadata(JNIEnv* env) {
    if (!env || !itemStackObj) return 0;
    Klass* itemStackKlass =
        Klass::Find(env, Mapping::Get("net/minecraft/item/ItemStack").c_str());
    if (!itemStackKlass) return 0;
    auto metadataField =
        itemStackKlass->GetField(env, Mapping::Get("metadata").c_str(), "I");
    if (!metadataField) return 0;
    return metadataField->GetIntField(env, itemStackObj);
}
