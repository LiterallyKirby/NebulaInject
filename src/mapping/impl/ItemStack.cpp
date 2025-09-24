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
        std::cerr << "[ItemStack::~ItemStack] Could not obtain JNIEnv to DeleteGlobalRef; leaking reference to avoid crash\n";
    }

    itemStackObj = nullptr;
}



// Return a local-ref jobject to the Java Item (not the ItemStack).
// The returned jobject is a local reference created by JNI CallObjectMethod and
// is valid in the current env; the caller may treat it as a local ref (we will
// delete it after use in IsWeapon).



jobject ItemStack::GetItem(JNIEnv* env)
{
    if (!env || !itemStackObj) return nullptr;

    const std::string itemStackClassName = Mapping::Get("net/minecraft/item/ItemStack");
    Klass* itemStackKlass = Klass::Find(env, itemStackClassName.c_str());
    if (!itemStackKlass) {
        std::cerr << "[ItemStack::GetItem] Failed to find class: " << itemStackClassName << "\n";
        return nullptr;
    }

    // Get method name and correct signature from mappings
    const std::string methodName   = Mapping::Get("getItem");         // likely "b"
    const std::string methodSig    = Mapping::Get("net/minecraft/item/Item", 3); // "()L...;"

    auto getItemMethod = itemStackKlass->GetMethod(env, methodName.c_str(), methodSig.c_str());
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
    if (!env || !itemStackObj) return false;

    jobject itemObj = GetItem(env);
    if (!itemObj) {
        std::cerr << "[ItemStack::IsWeapon] itemObj is null\n";
        return false;
    }

    const std::string swordClassName = Mapping::Get("net/minecraft/item/ItemSword");
    const std::string axeClassName   = Mapping::Get("net/minecraft/item/ItemAxe");

    Klass* swordKlass = nullptr;
    Klass* axeKlass   = nullptr;

    if (!swordClassName.empty()) {
        swordKlass = Klass::Find(env, swordClassName.c_str());
        if (!swordKlass)
            std::cerr << "[ItemStack::IsWeapon] Failed to find sword class: " << swordClassName << "\n";
    }

    if (!axeClassName.empty()) {
        axeKlass = Klass::Find(env, axeClassName.c_str());
        if (!axeKlass)
            std::cerr << "[ItemStack::IsWeapon] Failed to find axe class: " << axeClassName << "\n";
    }

    bool isSword = swordKlass && env->IsInstanceOf(itemObj, (jclass)swordKlass);
    bool isAxe   = axeKlass && env->IsInstanceOf(itemObj, (jclass)axeKlass);

    env->DeleteLocalRef(itemObj);
    return isSword || isAxe;
}



// Keep your other methods but ensure they operate on itemStackObj (global ref)
bool ItemStack::IsBlock(JNIEnv* env) {
    if (!env || !itemStackObj) return false;
    jobject itemObj = GetItem(env);
    if (!itemObj) return false;
    Klass* blockKlass =
        Klass::Find(env, Mapping::Get("net/minecraft/item/ItemBlock").c_str());
    bool res = blockKlass && env->IsInstanceOf(itemObj, (jclass)blockKlass);
    env->DeleteLocalRef(itemObj);
    return res;
}

bool ItemStack::IsEnderPearl(JNIEnv* env) {
    if (!env || !itemStackObj) return false;
    jobject itemObj = GetItem(env);
    if (!itemObj) return false;
    Klass* pearlKlass = Klass::Find(
        env, Mapping::Get("net/minecraft/item/ItemEnderPearl").c_str());
    bool res = pearlKlass && env->IsInstanceOf(itemObj, (jclass)pearlKlass);
    env->DeleteLocalRef(itemObj);
    return res;
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
