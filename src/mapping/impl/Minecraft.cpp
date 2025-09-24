#include "Minecraft.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"
#include <iostream>

extern Klass* g_Instance;

Minecraft::Minecraft(Phantom* phantom) {
    (void)phantom;
}


jobject Minecraft::GetTheMinecraft(JNIEnv* env)
{
    static int callCounter = 0;
    callCounter++;

    const bool debugThisCall = (callCounter % 500 == 0); // every 500 calls

    if (debugThisCall) {
        std::cout << "[DEBUG] Getting Minecraft instance... call #" << callCounter << std::endl;
    }

    if (!env) {
        std::cerr << "[ERROR] JNIEnv is null in GetTheMinecraft" << std::endl;
        return nullptr;
    }

    const std::string minecraftClassName = Mapping::Get("net/minecraft/client/Minecraft");
    if (debugThisCall) {
        std::cout << "[DEBUG] Minecraft class name: " << minecraftClassName << std::endl;
    }

    const auto clazz = Klass::Find(env, minecraftClassName.c_str());
    if (!clazz) {
        std::cerr << "[ERROR] Failed to find Minecraft class: " << minecraftClassName << std::endl;
        return nullptr;
    }

    const std::string theMinecraftFieldName = Mapping::Get("theMinecraft");
    const std::string theMinecraftFieldSig = Mapping::Get("net/minecraft/client/Minecraft", 2);

    if (debugThisCall) {
        std::cout << "[DEBUG] Looking for field: " << theMinecraftFieldName
                  << " with signature: " << theMinecraftFieldSig << std::endl;
    }

    const auto theMinecraftField = clazz->GetField(env,
        theMinecraftFieldName.c_str(),
        theMinecraftFieldSig.c_str(),
        true);

    if (!theMinecraftField) {
        std::cerr << "[ERROR] Failed to find theMinecraft field" << std::endl;
        return nullptr;
    }

    jobject minecraftInstance = theMinecraftField->GetObjectField(env, clazz, true);
    if (!minecraftInstance) {
        std::cerr << "[ERROR] Minecraft instance is null" << std::endl;
        return nullptr;
    }

    if (debugThisCall) {
        std::cout << "[DEBUG] Successfully got Minecraft instance" << std::endl;
    }

    return minecraftInstance;
}



jobject Minecraft::GetThePlayer(JNIEnv* env) {
    static jclass mcClass = nullptr;
    static jfieldID thePlayerField = nullptr;

    if (!mcClass) {
        std::string minecraftClassName = Mapping::Get("net/minecraft/client/Minecraft");
        mcClass = env->FindClass(minecraftClassName.c_str());
        if (!mcClass) {
            std::cerr << "[ERROR] Failed to find Minecraft class" << std::endl;
            return nullptr;
        }
        mcClass = (jclass)env->NewGlobalRef(mcClass);
    }

    if (!thePlayerField) {
        std::string fieldName = Mapping::Get("thePlayer");
        std::string fieldSig  = Mapping::Get("net/minecraft/client/entity/EntityClientPlayerMP", 2);
        thePlayerField = env->GetFieldID(mcClass, fieldName.c_str(), fieldSig.c_str());
        if (!thePlayerField) {
            std::cerr << "[ERROR] Failed to get thePlayer field ID" << std::endl;
            return nullptr;
        }
    }

    jobject minecraftInstance = GetTheMinecraft(env);
    if (!minecraftInstance) {
        std::cerr << "[ERROR] No Minecraft instance for GetThePlayer" << std::endl;
        return nullptr;
    }

    jobject playerObj = env->GetObjectField(minecraftInstance, thePlayerField);
    if (!playerObj) {
        std::cout << "[DEBUG] Player object is null (probably not in world)" << std::endl;
        return nullptr;
    }

    return playerObj;
}


jobject Minecraft::GetTheWorld(JNIEnv* env)
{

    
    jobject minecraftInstance = GetTheMinecraft(env);
    if (!minecraftInstance) {
        std::cerr << "[ERROR] No Minecraft instance for GetTheWorld" << std::endl;
        return nullptr;
    }

    const std::string minecraftClassName = Mapping::Get("net/minecraft/client/Minecraft");
    const auto clazz = Klass::Find(env, minecraftClassName.c_str());
    if (!clazz) {
        std::cerr << "[ERROR] Failed to find Minecraft class in GetTheWorld" << std::endl;
        return nullptr;
    }

    const std::string theWorldFieldName = Mapping::Get("theWorld");
    const std::string theWorldFieldSig = Mapping::Get("net/minecraft/client/multiplayer/WorldClient", 2);
    

    
    const auto theWorldField = clazz->GetField(env,
        theWorldFieldName.c_str(),
        theWorldFieldSig.c_str(),
        false); // instance field
        
    if (!theWorldField) {
        std::cerr << "[ERROR] Failed to find theWorld field" << std::endl;
        return nullptr;
    }

    jobject worldObj = theWorldField->GetObjectField(env, minecraftInstance, false);
    if (!worldObj) {

        return nullptr;
    }
    

    return worldObj;
}

// Rest of your methods with similar error handling...
jobject Minecraft::GetCurrentScreen(JNIEnv* env)
{
    const auto clazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto currentScreenField = clazz->GetField(env,
        Mapping::Get("currentScreen").c_str(),
        Mapping::Get("net/minecraft/client/gui/GuiScreen", 2).c_str(),
        false);
    return currentScreenField->GetObjectField(env, GetTheMinecraft(env), false);
}

jobject Minecraft::GetRenderManager(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto renderManagerClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/renderer/entity/RenderManager").c_str());

    if (g_GameVersion == CASUAL_1_7_10 || g_GameVersion == FORGE_1_7_10 || g_GameVersion == LUNAR_1_7_10) {
        const auto renderManagerInstanceField = renderManagerClazz->GetField(env,
            Mapping::Get("renderManagerInstance").c_str(),
            Mapping::Get("net/minecraft/client/renderer/entity/RenderManager", 2).c_str(),
            true);
        return renderManagerInstanceField->GetObjectField(env, renderManagerClazz, true);
    } else {
        const auto renderManagerInstanceMethod = minecraftClazz->GetMethod(env,
            Mapping::Get("getRenderManager").c_str(),
            Mapping::Get("net/minecraft/client/renderer/entity/RenderManager", 3).c_str());
        return renderManagerInstanceMethod->CallObjectMethod(env, GetTheMinecraft(env));
    }
}

jobject Minecraft::GetTimer(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto timerField = minecraftClazz->GetField(env,
        Mapping::Get("timer").c_str(),
        Mapping::Get("net/minecraft/util/Timer", 2).c_str());
    return timerField->GetObjectField(env, GetTheMinecraft(env));
}

jobject Minecraft::GetFontRenderer(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto fontRendererField = minecraftClazz->GetField(env,
        Mapping::Get("fontRendererObj").c_str(),
        Mapping::Get("net/minecraft/client/gui/FontRenderer", 2).c_str());
    return fontRendererField->GetObjectField(env, GetTheMinecraft(env));
}

jobject Minecraft::GetObjectMouseOver(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto objectMouseOverField = minecraftClazz->GetField(env,
        Mapping::Get("objectMouseOver").c_str(),
        Mapping::Get("net/minecraft/util/MovingObjectPosition", 2).c_str());
    return objectMouseOverField->GetObjectField(env, GetTheMinecraft(env));
}

jobject Minecraft::GetGameSettings(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto gameSettingsField = minecraftClazz->GetField(env,
        Mapping::Get("gameSettings").c_str(),
        Mapping::Get("net/minecraft/client/settings/GameSettings", 2).c_str());
    return gameSettingsField->GetObjectField(env, GetTheMinecraft(env));
}

int Minecraft::GetDisplayHeight(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto displayHeightField = minecraftClazz->GetField(env, Mapping::Get("displayHeight").c_str(), "I");
    return displayHeightField->GetIntField(env, GetTheMinecraft(env));
}

int Minecraft::GetDisplayWidth(JNIEnv* env)
{
    const auto minecraftClazz = Klass::Find(env, Mapping::Get("net/minecraft/client/Minecraft").c_str());
    const auto displayWidthField = minecraftClazz->GetField(env, Mapping::Get("displayWidth").c_str(), "I");
    return displayWidthField->GetIntField(env, GetTheMinecraft(env));
}
