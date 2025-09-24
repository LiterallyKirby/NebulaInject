
#include "EntityPlayerSP.h"

#include <cstring>
#include <string>
#include <vector>

#include "../Klass.h"
#include "../Mapping.h"

#include "../impl/AxisAlignedBB.h"
#include "../Method.h"
#include "ItemStack.h"

// In your Player.cpp file


// Player.cpp (constructor + destructor snippets)

#include <jni.h>
#include <iostream>

// add jvm member to Player class declaration if not present:
// JavaVM* jvm = nullptr;  // in Player.h

Player::Player(JNIEnv* env_, jobject obj)
    : env(env_), playerObj(nullptr), jvm(nullptr) {
    if (!env_ || !obj) return;

    // store the JavaVM pointer for safe cleanup from any thread later
    if (env_->GetJavaVM(&jvm) != JNI_OK) {
        std::cerr << "[Player] Warning: GetJavaVM failed\n";
        jvm = nullptr;
    }

    // Make a global ref so this wrapper can be used across JNI calls/threads
    // safely
    playerObj = env_->NewGlobalRef(obj);
    if (!playerObj) {
        std::cerr << "[Player] Failed to create global ref for Player\n";
    }
}



Player::~Player() {
    if (!playerObj) return;

    JNIEnv* env = nullptr;
    if (!jvm) {
        std::cerr << "[Player::~Player] No JavaVM available\n";
        return;
    }

    jint getEnvResult = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool attachedHere = false;

    if (getEnvResult == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != JNI_OK) {
            std::cerr << "[Player::~Player] Failed to attach thread\n";
            return;
        }
        attachedHere = true;
    } else if (getEnvResult != JNI_OK) {
        std::cerr << "[Player::~Player] Failed to get JNIEnv\n";
        return;
    }

    env->DeleteGlobalRef(playerObj);
    playerObj = nullptr;

    if (attachedHere) {
        jvm->DetachCurrentThread();
    }
}






AxisAlignedBB_t Player::GetBoundingBox() {
    if (!playerObj || !env) {
        return {};
    }

    // Get Player class and boundingBox field
    jclass playerClazz = env->GetObjectClass(playerObj);
    if (!playerClazz) return {};

    std::string aabbSig = "L" + Mapping::Get("net/minecraft/util/AxisAlignedBB") + ";";
    jfieldID boundingBoxField = env->GetFieldID(
        playerClazz,
        Mapping::Get("boundingBox").c_str(),
        aabbSig.c_str()
    );

    env->DeleteLocalRef(playerClazz);

    if (!boundingBoxField) {
        return {};
    }

    jobject aabbObj = env->GetObjectField(playerObj, boundingBoxField);
    if (!aabbObj) {
        return {};
    }

    // Wrap the Java AxisAlignedBB object safely
    AxisAlignedBB aabbWrapper(env, aabbObj);

    // We can delete the local reference now because the wrapper holds a global ref
    env->DeleteLocalRef(aabbObj);

    // Read bounding box values
    return aabbWrapper.GetNativeBoundingBox();
}



jobject Player::GetBoundingBoxJavaObject() {
    if (!playerObj || !env) return nullptr;
    
    jclass playerClazz = env->GetObjectClass(playerObj);
    if (!playerClazz) return nullptr;
    
    std::string aabbSig = "L" + Mapping::Get("net/minecraft/util/AxisAlignedBB") + ";";
    jfieldID boundingBoxField = env->GetFieldID(
        playerClazz,
        Mapping::Get("boundingBox").c_str(),  // "f"
        aabbSig.c_str()
    );
    
    if (!boundingBoxField) {
        env->DeleteLocalRef(playerClazz);
        return nullptr;
    }
    
    jobject aabbObj = env->GetObjectField(playerObj, boundingBoxField);
    env->DeleteLocalRef(playerClazz);
    
    return aabbObj; // Caller is responsible for cleanup
}

std::string Player::GetName(bool shouldEraseColor) {
    if (!playerObj || !env) return "";
    
    jclass playerClazz = env->GetObjectClass(playerObj);
    if (!playerClazz) return "";
    
    // Call getDisplayName() directly on the EntityPlayer object
    // Build the correct signature using the mapped IChatComponent class
    std::string chatComponentSig = "()L" + Mapping::Get("net/minecraft/util/IChatComponent") + ";";
    jmethodID getDisplayName = env->GetMethodID(
        playerClazz,
        Mapping::Get("getDisplayName").c_str(),  // "f_"
        chatComponentSig.c_str()  // "()Leu;"
    );
    
    if (!getDisplayName) {
        env->DeleteLocalRef(playerClazz);
        return "";
    }
    
    jobject chatComponent = env->CallObjectMethod(playerObj, getDisplayName);
    if (!chatComponent) {
        env->DeleteLocalRef(playerClazz);
        return "";
    }
    
    // Get unformatted text from IChatComponent
    jclass chatClazz = env->GetObjectClass(chatComponent);
    jmethodID getText = env->GetMethodID(
        chatClazz,
        Mapping::Get("getUnformattedTextForChat").c_str(), // "e"
        "()Ljava/lang/String;"
    );
    
    if (!getText) {
        env->DeleteLocalRef(playerClazz);
        env->DeleteLocalRef(chatComponent);
        env->DeleteLocalRef(chatClazz);
        return "";
    }
    
    jstring nameStr = (jstring)env->CallObjectMethod(chatComponent, getText);
    if (!nameStr) {
        env->DeleteLocalRef(playerClazz);
        env->DeleteLocalRef(chatComponent);
        env->DeleteLocalRef(chatClazz);
        return "";
    }
    
    const char* cStr = env->GetStringUTFChars(nameStr, nullptr);
    std::string ret(cStr ? cStr : "");
    env->ReleaseStringUTFChars(nameStr, cStr);
    
    // Cleanup
    env->DeleteLocalRef(playerClazz);
    env->DeleteLocalRef(chatComponent);
    env->DeleteLocalRef(chatClazz);
    env->DeleteLocalRef(nameStr);
    
    // Strip color formatting if requested
    if (shouldEraseColor) {
        for (size_t i = 0; i < ret.size(); ++i) {
            if ((unsigned char)ret[i] == 0xC2 && i + 1 < ret.size() &&
                (unsigned char)ret[i + 1] == 0xA7) {
                ret.erase(i, 2);
                if (i + 1 < ret.size()) {
                    ret.erase(i, 1); // Remove the color code character
                }
                --i;
            }
        }
    }
    
    return ret;
}


ItemStack* Player::GetHeldItem() {
    if (!playerObj || !env) {
        std::cerr << "[ERROR] Player or env is null in GetHeldItem"
                  << std::endl;
        return nullptr;
    }

    // Look up EntityClientPlayerMP (where thePlayer points to)
    const std::string playerClassName =
        Mapping::Get("net/minecraft/client/entity/EntityClientPlayerMP");
    const auto playerClazz = Klass::Find(env, playerClassName.c_str());
    if (!playerClazz) {
        std::cerr << "[ERROR] Failed to find EntityClientPlayerMP class"
                  << std::endl;
        return nullptr;
    }

    // Resolve method from mapping
    const std::string methodName = Mapping::Get("getHeldItem");
    const std::string methodSig =
        "()L" + Mapping::Get("net/minecraft/item/ItemStack") + ";";

    const auto method =
        playerClazz->GetMethod(env, methodName.c_str(), methodSig.c_str());
    if (!method) {
        std::cerr << "[ERROR] Could not resolve getHeldItem method (name="
                  << methodName << ", sig=" << methodSig << ")" << std::endl;
        return nullptr;
    }

    // Call method on playerObj
    jobject heldItemObj = method->CallObjectMethod(env, playerObj);
    if (!heldItemObj) {
        return nullptr;  // Player not holding anything
    }

    // Wrap result into ItemStack*
    return new ItemStack(env, heldItemObj);
}

float Player::GetRotationPitch() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("rotationPitch").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

int Player::GetMaxHurtResistantTime() {
    if (!playerObj || !env) return 0;

    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(
        clazz, Mapping::Get("maxHurtResistantTime").c_str(), "I");  // "I" = int
    int val = env->GetIntField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

int Player::GetHurtResistantTime() {
    if (!playerObj || !env) return 0;

    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(
        clazz, Mapping::Get("hurtResistantTime").c_str(), "I");  // "I" = int
    int val = env->GetIntField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRotationYaw() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("rotationYaw").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRotationYawHead() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("rotationYawHead").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRotationPitch() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("prevRotationPitch").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRotationYaw() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("prevRotationYaw").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRenderYawOffset() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(
        clazz, Mapping::Get("prevRenderYawOffset").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRenderYawOffset() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field =
        env->GetFieldID(clazz, Mapping::Get("renderYawOffset").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

Vec3D Player::GetLastTickPos() {
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("lastTickPosX").c_str(), "D"));
    double y = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("lastTickPosY").c_str(), "D"));
    double z = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("lastTickPosZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return {x, y, z};
}

Vec3D Player::GetPos() {
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("posX").c_str(), "D"));
    double y = env->GetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("posY").c_str(), "D"));
    double z = env->GetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("posZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return {x, y, z};
}

Vec3D Player::GetPreviousPos() {
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("prevPosX").c_str(), "D"));
    double y = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("prevPosY").c_str(), "D"));
    double z = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("prevPosZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return {x, y, z};
}

double Player::GetMotionX() {
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("motionX").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

double Player::GetMotionY() {
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("motionY").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

double Player::GetMotionZ() {
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("motionZ").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetHealth() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID getHealth =
        env->GetMethodID(clazz, Mapping::Get("getHealth").c_str(), "()F");
    float val = env->CallFloatMethod(playerObj, getHealth);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetMoveForward() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    float val = env->GetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("moveForward").c_str(), "F"));
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetMoveStrafing() {
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    float val = env->GetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("moveStrafing").c_str(), "F"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsSneaking() {
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID isSneaking =
        env->GetMethodID(clazz, Mapping::Get("isSneaking").c_str(), "()Z");
    bool val = env->CallBooleanMethod(playerObj, isSneaking);
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsOnGround() {
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    bool val = env->GetBooleanField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("onGround").c_str(), "Z"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsInWater() {
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    bool val = env->GetBooleanField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("inWater").c_str(), "Z"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsInvisible() {
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID isInvisible =
        env->GetMethodID(clazz, Mapping::Get("isInvisible").c_str(), "()Z");
    bool val = env->CallBooleanMethod(playerObj, isInvisible);
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsNPC() {
    if (!playerObj || !env) return true;
    std::string name = GetName();
    if (name.empty()) return true;
    if (name.find("[NPC]") != std::string::npos) return true;
    if (IsInvisible()) return true;
    return false;
}

// Setter methods
void Player::SetMotionX(double value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("motionX").c_str(), "D"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetMotionY(double value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("motionY").c_str(), "D"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetMotionZ(double value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(
        playerObj, env->GetFieldID(clazz, Mapping::Get("motionZ").c_str(), "D"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetRotationPitch(float value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("rotationPitch").c_str(), "F"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetRotationYaw(float value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("rotationYaw").c_str(), "F"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetPrevRotationPitch(float value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("prevRotationPitch").c_str(), "F"),
        value);
    env->DeleteLocalRef(clazz);
}

void Player::SetPrevRotationYaw(float value) {
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(
        playerObj,
        env->GetFieldID(clazz, Mapping::Get("prevRotationYaw").c_str(), "F"),
        value);
    env->DeleteLocalRef(clazz);
}
