#include "MovingObjectPosition.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

bool MovingObjectPosition::IsAimingBlock(JNIEnv* env)
{
    if (!env)
        return false;

    const std::string movingObjectClassName = Mapping::Get("net/minecraft/util/MovingObjectPosition");
    Klass* movingObjectClazz = Klass::Find(env, movingObjectClassName.c_str());
    if (!movingObjectClazz)
        return false;

    const auto typeOfHitField = movingObjectClazz->GetField(
        env,
        Mapping::Get("typeOfHit").c_str(),
        Mapping::Get("net/minecraft/util/MovingObjectPosition$MovingObjectType", 2).c_str()
    );
    if (!typeOfHitField)
        return false;

    jobject typeOfHitObject = typeOfHitField->GetObjectField(env, (jobject)this);
    if (!typeOfHitObject)
        return false;

    Klass* movingObjectTypeClazz = (Klass*)env->GetObjectClass(typeOfHitObject);
    const auto enumBLOCKField = movingObjectTypeClazz->GetField(
        env,
        Mapping::Get("BLOCK").c_str(),
        Mapping::Get("net/minecraft/util/MovingObjectPosition$MovingObjectType", 2).c_str(),
        true
    );
    if (!enumBLOCKField)
        return false;

    jobject obj = enumBLOCKField->GetObjectField(env, movingObjectTypeClazz, true);
    if (!obj)
        return false;

    return env->IsSameObject(obj, typeOfHitObject);
}

bool MovingObjectPosition::IsAimingEntity(JNIEnv* env)
{
    if (!env)
        return false;

    const std::string movingObjectClassName = Mapping::Get("net/minecraft/util/MovingObjectPosition");
    Klass* movingObjectClazz = Klass::Find(env, movingObjectClassName.c_str());
    if (!movingObjectClazz)
        return false;

    const auto typeOfHitField = movingObjectClazz->GetField(
        env,
        Mapping::Get("typeOfHit").c_str(),
        Mapping::Get("net/minecraft/util/MovingObjectPosition$MovingObjectType", 2).c_str()
    );
    if (!typeOfHitField)
        return false;

    jobject typeOfHitObject = typeOfHitField->GetObjectField(env, (jobject)this);
    if (!typeOfHitObject)
        return false;

    Klass* movingObjectTypeClazz = (Klass*)env->GetObjectClass(typeOfHitObject);
    const auto enumENTITYField = movingObjectTypeClazz->GetField(
        env,
        Mapping::Get("ENTITY").c_str(),
        Mapping::Get("net/minecraft/util/MovingObjectPosition$MovingObjectType", 2).c_str(),
        true
    );
    if (!enumENTITYField)
        return false;

    jobject obj = enumENTITYField->GetObjectField(env, movingObjectTypeClazz, true);
    if (!obj)
        return false;

    return env->IsSameObject(obj, typeOfHitObject);
}
