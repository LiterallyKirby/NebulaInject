//
// Some of this code was copied from UDP-CPP: https://github.com/UnknownDetectionParty/UDP-CPP
//

#include "Minecraft.h"
#include <net/minecraft/entity/EntityPlayerSP.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>

Minecraft::Minecraft(Phantom *phantom) : AbstractClass::AbstractClass(phantom, "Minecraft") {
    // Initialize all field/method IDs to nullptr first
    smdGetMinecraft = nullptr;
    fdPlayer = fdWorld = fdObjectMouseOver = fdPointedEntity = nullptr;
    fdGameSettings = fdInGameHasFocus = fdTimer = fdEntityRenderer = nullptr;
    fdRightClickDelayTimer = fdLeftClickMouse = nullptr;
    mdGetRenderViewEntity = nullptr;
    
    // Get method/field IDs
    smdGetMinecraft = getMethodID("getMinecraft");
    fdPlayer = getFieldID("player");
    fdWorld = getFieldID("world");
    fdObjectMouseOver = getFieldID("objectMouseOver");
    fdPointedEntity = getFieldID("pointedEntity");
    fdGameSettings = getFieldID("gameSettings");
    fdInGameHasFocus = getFieldID("inGameHasFocus");
    fdTimer = getFieldID("timer");
    fdEntityRenderer = getFieldID("entityRenderer");
    fdRightClickDelayTimer = getFieldID("rightClickDelayTimer");
    fdLeftClickMouse = getFieldID("leftClickMouse");
    mdGetRenderViewEntity = getMethodID("getRenderViewEntity");

    // init reflection backup
    reflectiveObjectMouseOverField = nullptr;
    useReflectionForObjectMouseOver = false;

    // If the direct field lookup failed, try reflection
    if (fdObjectMouseOver == nullptr) {
        JNIEnv *env = phantom->getEnv();
        if (env) {
            jclass mcClass = env->FindClass("net/minecraft/client/Minecraft");
            if (mcClass) {
                findObjectMouseOverFieldViaReflection(env, mcClass);
                env->DeleteLocalRef(mcClass);
            }
        }
    }
}

// Add destructor to clean up global references
Minecraft::~Minecraft() {
    if (reflectiveObjectMouseOverField) {
        JNIEnv *env = phantom->getEnv();
        if (env) {
            env->DeleteGlobalRef(reflectiveObjectMouseOverField);
            reflectiveObjectMouseOverField = nullptr;
        }
    }
}

void Minecraft::findObjectMouseOverFieldViaReflection(JNIEnv *env, jclass mcClass) {
    jclass classClass = env->FindClass("java/lang/Class");
    jclass fieldClass = env->FindClass("java/lang/reflect/Field");
    if (!classClass || !fieldClass) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (classClass) env->DeleteLocalRef(classClass);
        if (fieldClass) env->DeleteLocalRef(fieldClass);
        return;
    }

    jmethodID getDeclaredFieldsMID = env->GetMethodID(classClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
    jmethodID setAccessibleMID = env->GetMethodID(fieldClass, "setAccessible", "(Z)V");
    jmethodID getNameMID = env->GetMethodID(fieldClass, "getName", "()Ljava/lang/String;");
    jmethodID getTypeMID = env->GetMethodID(fieldClass, "getType", "()Ljava/lang/Class;");

    if (!getDeclaredFieldsMID || !setAccessibleMID || !getNameMID || !getTypeMID) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(fieldClass);
        return;
    }

    jobjectArray fields = (jobjectArray) env->CallObjectMethod(mcClass, getDeclaredFieldsMID);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(fieldClass);
        return;
    }

    jsize len = env->GetArrayLength(fields);
    for (jsize i = 0; i < len; ++i) {
        jobject field = env->GetObjectArrayElement(fields, i);
        if (!field) continue;

        jobject typeClass = env->CallObjectMethod(field, getTypeMID);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(field);
            continue;
        }

        jmethodID getDeclaredFieldMID = env->GetMethodID(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
        if (!getDeclaredFieldMID) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(typeClass);
            env->DeleteLocalRef(field);
            continue;
        }

        jstring typeOfHitName = env->NewStringUTF("typeOfHit");
        jobject found = env->CallObjectMethod(typeClass, getDeclaredFieldMID, typeOfHitName);

        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(typeOfHitName);
            env->DeleteLocalRef(typeClass);
            env->DeleteLocalRef(field);
            continue;
        }

        // Found the field - make it accessible and store global reference
        env->CallVoidMethod(field, setAccessibleMID, JNI_TRUE);
        if (env->ExceptionCheck()) env->ExceptionClear();

        reflectiveObjectMouseOverField = env->NewGlobalRef(field);
        useReflectionForObjectMouseOver = true;

        // cleanup
        env->DeleteLocalRef(found);
        env->DeleteLocalRef(typeOfHitName);
        env->DeleteLocalRef(typeClass);
        env->DeleteLocalRef(field);
        break;
    }

    env->DeleteLocalRef(fields);
    env->DeleteLocalRef(classClass);
    env->DeleteLocalRef(fieldClass);
}

jobject Minecraft::getObjectMouseOver() {
    // Fast path: direct field id
    if (fdObjectMouseOver != nullptr) {
        jobject mc = getMinecraft();
        return mc ? getObject(mc, fdObjectMouseOver) : nullptr;
    }

    // Reflection fallback
    if (useReflectionForObjectMouseOver && reflectiveObjectMouseOverField != nullptr) {
        JNIEnv *env = phantom->getEnv();
        if (!env) return nullptr;

        jclass fieldClass = env->FindClass("java/lang/reflect/Field");
        if (!fieldClass) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            return nullptr;
        }
        
        jmethodID getMID = env->GetMethodID(fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        if (!getMID) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(fieldClass);
            return nullptr;
        }

        jobject mc = getMinecraft();
        if (!mc) {
            env->DeleteLocalRef(fieldClass);
            return nullptr;
        }

        jobject value = env->CallObjectMethod(reflectiveObjectMouseOverField, getMID, mc);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(fieldClass);
            return nullptr;
        }

        env->DeleteLocalRef(fieldClass);
        return value;
    }

    return nullptr;
}

jobject Minecraft::getMinecraft() {
    return smdGetMinecraft ? getObject(smdGetMinecraft) : nullptr;
}

jobject Minecraft::getPlayer() {
    jobject mc = getMinecraft();
    return (mc && fdPlayer) ? getObject(mc, fdPlayer) : nullptr;
}

jobject Minecraft::getWorld() {
    jobject mc = getMinecraft();
    return (mc && fdWorld) ? getObject(mc, fdWorld) : nullptr;
}

jobject Minecraft::getGameSettings() {
    jobject mc = getMinecraft();
    return (mc && fdGameSettings) ? getObject(mc, fdGameSettings) : nullptr;
}

jboolean Minecraft::isInGameHasFocus() {
    jobject mc = getMinecraft();
    return (mc && fdInGameHasFocus) ? getBoolean(mc, fdInGameHasFocus) : JNI_FALSE;
}

jobject Minecraft::getRenderViewEntity() {
    jobject mc = getMinecraft();
    return (mc && mdGetRenderViewEntity) ? getObject(mc, mdGetRenderViewEntity) : nullptr;
}

void Minecraft::setObjectMouseOver(jobject object) {
    jobject mc = getMinecraft();
    if (mc && fdObjectMouseOver) {
        setObject(mc, fdObjectMouseOver, object);
    }
}

jobject Minecraft::getPointedEntity() {
    jobject mc = getMinecraft();
    return (mc && fdPointedEntity) ? getObject(mc, fdPointedEntity) : nullptr;
}

void Minecraft::setPointedEntity(jobject object) {
    jobject mc = getMinecraft();
    if (mc && fdPointedEntity) {
        setObject(mc, fdPointedEntity, object);
    }
}

jobject Minecraft::getTimer() {
    jobject mc = getMinecraft();
    return (mc && fdTimer) ? getObject(mc, fdTimer) : nullptr;
}

jobject Minecraft::getEntityRenderer() {
    jobject mc = getMinecraft();
    return (mc && fdEntityRenderer) ? getObject(mc, fdEntityRenderer) : nullptr;
}

jint Minecraft::getRightClickDelayTimer() {
    jobject mc = getMinecraft();
    return (mc && fdRightClickDelayTimer) ? getInt(mc, fdRightClickDelayTimer) : 0;
}

void Minecraft::setRightClickDelayTimer(jint rightClickDelayTimer) {
    jobject mc = getMinecraft();
    if (mc && fdRightClickDelayTimer) {
        setInt(mc, fdRightClickDelayTimer, rightClickDelayTimer);
    }
}

jint Minecraft::getLeftClickMouse() {
    jobject mc = getMinecraft();
    return (mc && fdLeftClickMouse) ? getInt(mc, fdLeftClickMouse) : 0;
}

void Minecraft::setLeftClickMouse(jint leftClickMouse) {
    jobject mc = getMinecraft();
    if (mc && fdLeftClickMouse) {
        setInt(mc, fdLeftClickMouse, leftClickMouse);
    }
}

EntityPlayerSP Minecraft::getPlayerContainer() {
    return EntityPlayerSP(phantom, this);
}

WorldClient Minecraft::getWorldContainer() {
    return WorldClient(phantom, this);
}

GameSettings Minecraft::getGameSettingsContainer() {
    return GameSettings(phantom, getGameSettings());
}

Entity Minecraft::getRenderViewEntityContainer() {
    return Entity(phantom, getRenderViewEntity());
}

Timer Minecraft::getTimerContainer() {
    return Timer(phantom, getTimer());
}

EntityRenderer Minecraft::getEntityRendererContainer() {
    return EntityRenderer(phantom, getEntityRenderer());
}

Phantom *Minecraft::getPhantom() {
    return phantom;
}
