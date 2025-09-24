
#include "EntityPlayerSP.h"
#include "../Mapping.h"
#include <string>
#include <vector>
#include <cstring>




std::string Player::GetName(bool shouldEraseColor) {
    if (!playerObj || !env) return "";

    jclass playerClazz = env->GetObjectClass(playerObj);
    if (!playerClazz) return "";

 
jmethodID getDisplayName = env->GetMethodID(
    playerClazz,
    Mapping::Get("getDisplayName").c_str(),
    "()Lnet/minecraft/util/IChatComponent;"  // <- correct JNI signature
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

    jclass chatClazz = env->FindClass(Mapping::Get("net/minecraft/util/IChatComponent").c_str());
    jmethodID getText = env->GetMethodID(chatClazz,
                                         Mapping::Get("getUnformattedTextForChat").c_str(),
                                         "()Ljava/lang/String;");
    jstring nameStr = (jstring)env->CallObjectMethod(chatComponent, getText);

    const char* cStr = env->GetStringUTFChars(nameStr, nullptr);
    std::string ret(cStr ? cStr : "");
    env->ReleaseStringUTFChars(nameStr, cStr);

    env->DeleteLocalRef(playerClazz);
    env->DeleteLocalRef(chatComponent);
    env->DeleteLocalRef(nameStr);
    env->DeleteLocalRef(chatClazz);


for (size_t i = 0; i < ret.size(); ++i) {
    if ((unsigned char)ret[i] == 0xC2 && i + 1 < ret.size() && (unsigned char)ret[i+1] == 0xA7) {
        ret.erase(i, 2); // erase the two bytes representing § in UTF-8
        --i;
    }
}


    return ret;
}


float Player::GetRotationPitch()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("rotationPitch").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRotationYaw()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("rotationYaw").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRotationYawHead()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("rotationYawHead").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRotationPitch()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("prevRotationPitch").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRotationYaw()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("prevRotationYaw").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetPrevRenderYawOffset()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("prevRenderYawOffset").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetRenderYawOffset()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jfieldID field = env->GetFieldID(clazz, Mapping::Get("renderYawOffset").c_str(), "F");
    float val = env->GetFloatField(playerObj, field);
    env->DeleteLocalRef(clazz);
    return val;
}

Vec3D Player::GetLastTickPos()
{
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("lastTickPosX").c_str(), "D"));
    double y = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("lastTickPosY").c_str(), "D"));
    double z = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("lastTickPosZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return { x, y, z };
}

Vec3D Player::GetPos()
{
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("posX").c_str(), "D"));
    double y = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("posY").c_str(), "D"));
    double z = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("posZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return { x, y, z };
}

Vec3D Player::GetPreviousPos()
{
    if (!playerObj || !env) return Vec3D();
    jclass clazz = env->GetObjectClass(playerObj);

    double x = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("prevPosX").c_str(), "D"));
    double y = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("prevPosY").c_str(), "D"));
    double z = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("prevPosZ").c_str(), "D"));

    env->DeleteLocalRef(clazz);
    return { x, y, z };
}

double Player::GetMotionX()
{
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionX").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

double Player::GetMotionY()
{
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionY").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

double Player::GetMotionZ()
{
    if (!playerObj || !env) return 0.0;
    jclass clazz = env->GetObjectClass(playerObj);
    double val = env->GetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionZ").c_str(), "D"));
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetHealth()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID getHealth = env->GetMethodID(clazz, Mapping::Get("getHealth").c_str(), "()F");
    float val = env->CallFloatMethod(playerObj, getHealth);
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetMoveForward()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    float val = env->GetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("moveForward").c_str(), "F"));
    env->DeleteLocalRef(clazz);
    return val;
}

float Player::GetMoveStrafing()
{
    if (!playerObj || !env) return 0.0f;
    jclass clazz = env->GetObjectClass(playerObj);
    float val = env->GetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("moveStrafing").c_str(), "F"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsSneaking()
{
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID isSneaking = env->GetMethodID(clazz, Mapping::Get("isSneaking").c_str(), "()Z");
    bool val = env->CallBooleanMethod(playerObj, isSneaking);
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsOnGround()
{
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    bool val = env->GetBooleanField(playerObj, env->GetFieldID(clazz, Mapping::Get("onGround").c_str(), "Z"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsInWater()
{
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    bool val = env->GetBooleanField(playerObj, env->GetFieldID(clazz, Mapping::Get("inWater").c_str(), "Z"));
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsInvisible()
{
    if (!playerObj || !env) return false;
    jclass clazz = env->GetObjectClass(playerObj);
    jmethodID isInvisible = env->GetMethodID(clazz, Mapping::Get("isInvisible").c_str(), "()Z");
    bool val = env->CallBooleanMethod(playerObj, isInvisible);
    env->DeleteLocalRef(clazz);
    return val;
}

bool Player::IsNPC()
{
    if (!playerObj || !env) return true;
    std::string name = GetName();
    if (name.empty()) return true;
    if (name.find("[NPC]") != std::string::npos) return true;
    if (IsInvisible()) return true;
    return false;
}

// Setter methods
void Player::SetMotionX(double value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionX").c_str(), "D"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetMotionY(double value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionY").c_str(), "D"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetMotionZ(double value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetDoubleField(playerObj, env->GetFieldID(clazz, Mapping::Get("motionZ").c_str(), "D"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetRotationPitch(float value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("rotationPitch").c_str(), "F"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetRotationYaw(float value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("rotationYaw").c_str(), "F"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetPrevRotationPitch(float value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("prevRotationPitch").c_str(), "F"), value);
    env->DeleteLocalRef(clazz);
}

void Player::SetPrevRotationYaw(float value)
{
    if (!playerObj || !env) return;
    jclass clazz = env->GetObjectClass(playerObj);
    env->SetFloatField(playerObj, env->GetFieldID(clazz, Mapping::Get("prevRotationYaw").c_str(), "F"), value);
    env->DeleteLocalRef(clazz);
}

