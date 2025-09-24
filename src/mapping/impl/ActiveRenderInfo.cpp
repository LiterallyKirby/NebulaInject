#include "ActiveRenderInfo.h"

#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

std::vector<float> ActiveRenderInfo::GetProjection(JNIEnv* env)
{
    // Convert Mapping::Get result to const char* for Klass::Find
    const auto activeRenderInfoClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/ActiveRenderInfo").c_str());

    const auto projectionField = activeRenderInfoClazz->GetField(env, Mapping::Get("PROJECTION").c_str(), "Ljava/nio/FloatBuffer;", true);
    const auto projectionObject = projectionField->GetObjectField(env, nullptr, true); // static field

    const auto floatBufferClazz = Klass::Find(env, "java/nio/FloatBuffer");
    const auto getMethod = floatBufferClazz->GetMethod(env, "get", "(I)F");

    std::vector<float> ret;
    for (int i = 0; i < 16; i++) {
        ret.push_back(getMethod->CallFloatMethod(env, projectionObject, false, i));
    }

    if (projectionObject)
        env->DeleteLocalRef((jobject)projectionObject);
    if (activeRenderInfoClazz)
        env->DeleteLocalRef((jclass)activeRenderInfoClazz);
    if (floatBufferClazz)
        env->DeleteLocalRef((jclass)floatBufferClazz);

    return ret;
}

std::vector<float> ActiveRenderInfo::GetModelView(JNIEnv* env)
{
    const auto activeRenderInfoClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/ActiveRenderInfo").c_str());

    const auto modelViewField = activeRenderInfoClazz->GetField(env, Mapping::Get("MODELVIEW").c_str(), "Ljava/nio/FloatBuffer;", true);
    const auto modelViewObject = modelViewField->GetObjectField(env, nullptr, true);

    const auto floatBufferClazz = Klass::Find(env, "java/nio/FloatBuffer");
    const auto getMethod = floatBufferClazz->GetMethod(env, "get", "(I)F");

    std::vector<float> ret;
    for (int i = 0; i < 16; i++) {
        ret.push_back(getMethod->CallFloatMethod(env, modelViewObject, false, i));
    }

    if (modelViewObject)
        env->DeleteLocalRef((jobject)modelViewObject);
    if (activeRenderInfoClazz)
        env->DeleteLocalRef((jclass)activeRenderInfoClazz);
    if (floatBufferClazz)
        env->DeleteLocalRef((jclass)floatBufferClazz);

    return ret;
}
