#include "ItemStack.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

jobject ItemStack::GetItem(JNIEnv* env)
{
    if (!env)
        return nullptr;

    Klass* itemStackClass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemStack").c_str());
    if (!itemStackClass)
        return nullptr;

    auto getItemMethod = itemStackClass->GetMethod(
        env,
        Mapping::Get("getItem").c_str(),
        Mapping::Get("net/minecraft/item/Item", 3).c_str()
    );
    if (!getItemMethod)
        return nullptr;

    return getItemMethod->CallObjectMethod(env, (jobject)this);
}

bool ItemStack::IsWeapon(JNIEnv* env)
{
    if (!env)
        return false;

    jobject heldItem = GetItem(env);
    if (!heldItem)
        return false;

    Klass* axeKlass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemAxe").c_str());
    Klass* swordKlass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemSword").c_str());
    if (!axeKlass || !swordKlass)
        return false;

    return env->IsInstanceOf(heldItem, (jclass)axeKlass) ||
           env->IsInstanceOf(heldItem, (jclass)swordKlass);
}

bool ItemStack::IsBlock(JNIEnv* env)
{
    if (!env)
        return false;

    jobject heldItem = GetItem(env);
    if (!heldItem)
        return false;

    Klass* blockKlass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemBlock").c_str());
    if (!blockKlass)
        return false;

    return env->IsInstanceOf(heldItem, (jclass)blockKlass);
}

bool ItemStack::IsEnderPearl(JNIEnv* env)
{
    if (!env)
        return false;

    jobject heldItem = GetItem(env);
    if (!heldItem)
        return false;

    Klass* pearlKlass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemEnderPearl").c_str());
    if (!pearlKlass)
        return false;

    return env->IsInstanceOf(heldItem, (jclass)pearlKlass);
}

bool ItemStack::Is(const char* clazz, JNIEnv* env)
{
    if (!env)
        return false;

    jobject heldItem = GetItem(env);
    if (!heldItem)
        return false;

    Klass* klass = Klass::Find(env, clazz);
    if (!klass)
        return false;

    return env->IsInstanceOf(heldItem, (jclass)klass);
}

int ItemStack::GetMetadata(JNIEnv* env)
{
    if (!env)
        return 0;

    Klass* itemStackClass = Klass::Find(env, Mapping::Get("net/minecraft/item/ItemStack").c_str());
    if (!itemStackClass)
        return 0;

    auto metadataField = itemStackClass->GetField(env, Mapping::Get("metadata").c_str(), "I");
    if (!metadataField)
        return 0;

    return metadataField->GetIntField(env, (jobject)this);
}
