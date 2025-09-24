#include "RenderManager.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

jobject RenderManager::getEntityRender(jobject argEntity, JNIEnv* env)
{
    if (!env)
        return NULL;
    
    const auto renderManagerClazz = (Klass*)env->GetObjectClass((jobject)this);
    const auto getEntityRenderMethod = renderManagerClazz->GetMethod(env, Mapping::Get("getEntityRenderObject").data(),
        std::string("(" + Mapping::Get("net/minecraft/entity/Entity", 2) + ")" + Mapping::Get("net/minecraft/client/renderer/entity/Render", 2)).data());
    
    if (renderManagerClazz)
        env->DeleteLocalRef((jclass)renderManagerClazz);
    
    return getEntityRenderMethod->CallObjectMethod(env, this, false, argEntity);
}

Vec3D RenderManager::GetRenderPos(JNIEnv* env)
{
    if (!env)
        return Vec3D();
    
    // Use Klass::Find instead of g_Instance->FindClass
    const auto renderManagerClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/entity/RenderManager").c_str());
    if (!renderManagerClazz) {
        return Vec3D();
    }
    
    if (g_GameVersion == CASUAL_1_7_10 || g_GameVersion == FORGE_1_7_10 || g_GameVersion == LUNAR_1_7_10) {
        const auto renderPosFieldX = renderManagerClazz->GetField(env, Mapping::Get("renderPosX").data(), "D", true);
        const auto renderPosFieldY = renderManagerClazz->GetField(env, Mapping::Get("renderPosY").data(), "D", true);
        const auto renderPosFieldZ = renderManagerClazz->GetField(env, Mapping::Get("renderPosZ").data(), "D", true);
        return Vec3D{ renderPosFieldX->GetDoubleField(env, (jobject)this, true), 
            renderPosFieldY->GetDoubleField(env, (jobject)this, true),
            renderPosFieldZ->GetDoubleField(env, (jobject)this, true)
        };
    }
    else {
        const auto renderPosFieldX = renderManagerClazz->GetField(env, Mapping::Get("renderPosX").data(), "D");
        const auto renderPosFieldY = renderManagerClazz->GetField(env, Mapping::Get("renderPosY").data(), "D");
        const auto renderPosFieldZ = renderManagerClazz->GetField(env, Mapping::Get("renderPosZ").data(), "D");
        return Vec3D{ renderPosFieldX->GetDoubleField(env, (jobject)this), 
            renderPosFieldY->GetDoubleField(env, (jobject)this), 
            renderPosFieldZ->GetDoubleField(env, (jobject)this) 
        };
    }
    
    return Vec3D();
}

Vec3D RenderManager::GetViewerPos(JNIEnv* env)
{
    if (!env)
        return Vec3D();
    
    // Use Klass::Find instead of g_Instance->FindClass
    const auto renderManagerClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/entity/RenderManager").c_str());
    if (!renderManagerClazz) {
        return Vec3D();
    }
    
    const auto fieldX = renderManagerClazz->GetField(env, Mapping::Get("viewerPosX").data(), "D");
    const auto fieldY = renderManagerClazz->GetField(env, Mapping::Get("viewerPosY").data(), "D");
    const auto fieldZ = renderManagerClazz->GetField(env, Mapping::Get("viewerPosZ").data(), "D");
    
    return Vec3D(fieldX->GetDoubleField(env, (jobject)this),
        fieldY->GetDoubleField(env, (jobject)this),
        fieldZ->GetDoubleField(env, (jobject)this)
    );
}

bool RenderManager::DoRenderEntity(jobject entity, double x, double y, double z, float entityYaw, float partialTicks, bool p_147939_10_, JNIEnv* env)
{
    if (!env)
        return false;
    
    // Use Klass::Find instead of g_Instance->FindClass
    const auto renderManagerClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/entity/RenderManager").c_str());
    if (!renderManagerClazz) {
        return false;
    }
    
    const auto doRenderEntityMethod = renderManagerClazz->GetMethod(env, "doRenderEntity", "(Lnet/minecraft/entity/Entity;DDDFFZ)Z");
    if (!doRenderEntityMethod) {
        return false;
    }
    
    return doRenderEntityMethod->CallBoolMethod(env, (jobject)this, false, entity, x, y, z, entityYaw, partialTicks, p_147939_10_);
}
