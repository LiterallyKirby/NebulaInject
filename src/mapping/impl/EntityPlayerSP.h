#pragma once

#include <jni.h>
#include "../Vec3.h"
#include "ItemStack.h"

#include "../impl/AxisAlignedBB.h"
#include <string>

class Player {
public:
    // ctor/dtor defined in Player.cpp
    Player(JNIEnv* env, jobject playerObj);
    ~Player();

    // Basic accessors
    std::string GetName(bool shouldEraseColor = false);
    float GetRotationPitch();
    float GetRotationYaw();
    AxisAlignedBB_t GetBoundingBox();
    float GetRotationYawHead();
    float GetPrevRotationPitch();
    float GetPrevRotationYaw();
    float GetPrevRenderYawOffset();
    ItemStack* GetHeldItem();
    float GetRenderYawOffset();
    float GetHealth();

    jobject GetJObject() const { return playerObj; }
    jobject GetBoundingBoxJavaObject();

    float GetMoveForward();
    float GetMoveStrafing();
    Vec3D GetLastTickPos();
    Vec3D GetPos();
    Vec3D GetPreviousPos();
    double GetMotionX();
    double GetMotionY();
    double GetMotionZ();
    bool IsNPC();
    bool IsInvisible();
    bool IsSneaking();
    bool IsOnGround();
    bool IsInWater();
    int GetMaxHurtResistantTime();
    int GetHurtResistantTime();

    jobject GetInventoryPlayer();

    void SetAlwaysRenderNameTag(bool state);
    void SetMotionX(double buffer);
    void SetMotionY(double buffer);
    void SetMotionZ(double buffer);
    void SetRotationPitch(float buffer);
    void SetRotationYaw(float buffer);
    void SetPrevRotationPitch(float buffer);
    void SetPrevRotationYaw(float buffer);

private:
    // Important: store both env (for convenience) and the JVM pointer
    // so the destructor can safely attach if called from another thread.
    JNIEnv* env = nullptr;
    jobject playerObj = nullptr; // global ref, managed in cpp
    JavaVM* jvm = nullptr;
};
