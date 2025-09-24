#pragma once
#include <vector>
#include "jni.h"
class Player;
class World
{
private:
    JNIEnv* env;
    jobject worldObj;
    
public:
    World(JNIEnv* env, jobject worldObj) : env(env), worldObj(worldObj) {
        if (worldObj)
            this->worldObj = env->NewGlobalRef(worldObj);
    }
    
    ~World() {
        if (worldObj)
            env->DeleteGlobalRef(worldObj);
    }
    
    std::vector<Player*> GetPlayerEntities(JNIEnv* env);
    
    // Add a method to check if world is valid
    bool isValid() const {
        return worldObj != nullptr;
    }
};
