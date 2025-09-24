#pragma once
#include "jni.h"
#include "../Vec3.h"
#include "string"
class Player
{
private:
    JNIEnv* env;
    jobject playerObj; // underlying Java player object

public:
    Player(JNIEnv* env, jobject playerObj) : env(env), playerObj(playerObj) {
        if (playerObj)
            this->playerObj = env->NewGlobalRef(playerObj); // keep global ref
    }

    ~Player() {
        if (playerObj)
            env->DeleteGlobalRef(playerObj);
    }

    std::string GetName(bool shouldEraseColor = false);
    float GetRotationPitch();
    float GetRotationYaw();
    float GetRotationYawHead();
    float GetPrevRotationPitch();
    float GetPrevRotationYaw();
    float GetPrevRenderYawOffset();
    float GetRenderYawOffset();
    float GetHealth();
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
    jobject GetBoundingBox();
    jobject GetHeldItem();
    jobject GetInventoryPlayer();

    void SetAlwaysRenderNameTag(bool state);
    void SetMotionX(double buffer);
    void SetMotionY(double buffer);
    void SetMotionZ(double buffer);
    void SetRotationPitch(float buffer);
    void SetRotationYaw(float buffer);
    void SetPrevRotationPitch(float buffer);
    void SetPrevRotationYaw(float buffer);
};
