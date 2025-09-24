#include "InventoryPlayer.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

int InventoryPlayer::GetSlot(JNIEnv* env)
{
    if (!this || !env) return 0;

    const auto inventoryPlayerClass = (Klass*)env->GetObjectClass((jobject)this);
    if (!inventoryPlayerClass) return 0;

    const auto hotbarSlotField = inventoryPlayerClass->GetField(env, Mapping::Get("currentItem").c_str(), "I");
    int slot = hotbarSlotField->GetIntField(env, this);

    env->DeleteLocalRef((jclass)inventoryPlayerClass);
    return slot;
}

void InventoryPlayer::SetSlot(int slot, JNIEnv* env)
{
    if (!this || !env) return;

    const auto inventoryPlayerClass = (Klass*)env->GetObjectClass((jobject)this);
    if (!inventoryPlayerClass) return;

    const auto hotbarSlotField = inventoryPlayerClass->GetField(env, Mapping::Get("currentItem").c_str(), "I");
    hotbarSlotField->SetIntField(env, this, slot);

    env->DeleteLocalRef((jclass)inventoryPlayerClass);
}

jobject InventoryPlayer::GetStackInSlot(int slot, JNIEnv* env)
{
    if (!this || !env) return nullptr;

    const auto inventoryPlayerClass = (Klass*)env->GetObjectClass((jobject)this);
    if (!inventoryPlayerClass) return nullptr;

    std::string sig = "(I)" + Mapping::Get("net/minecraft/item/ItemStack", 2);
    const auto getStackInSlotMethod = inventoryPlayerClass->GetMethod(env, Mapping::Get("getStackInSlot").c_str(), sig.c_str());

    jobject stack = getStackInSlotMethod->CallObjectMethod(env, (jobject)this, slot);

    env->DeleteLocalRef((jclass)inventoryPlayerClass);
    return stack;
}
