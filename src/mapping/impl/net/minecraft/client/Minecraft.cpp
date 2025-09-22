#include "Minecraft.h"
#include <net/minecraft/entity/EntityPlayerSP.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>

// Updated Minecraft constructor using CM system
Minecraft::Minecraft(Phantom *phantom) : AbstractClass::AbstractClass(phantom, "Minecraft") {
    // Initialize all field/method IDs to nullptr first
    smdGetMinecraft = nullptr;
    fdPlayer = fdWorld = fdObjectMouseOver = fdPointedEntity = nullptr;
    fdGameSettings = fdInGameHasFocus = fdTimer = fdEntityRenderer = nullptr;
    fdRightClickDelayTimer = fdLeftClickMouse = nullptr;
    mdGetRenderViewEntity = nullptr;
    
    // Get the mapped class name
    CM* mcClass = Mapping::getClass("net/minecraft/client/Minecraft");
    if (!mcClass) {
        std::cerr << "[Minecraft ERROR] Could not find Minecraft class mapping" << std::endl;
        return;
    }
    
    // Update the class name in AbstractClass

    
    // Get method/field IDs using CM system
    std::string staticMethodName = Mapping::getMethodName("net/minecraft/client/Minecraft", "theMinecraft");
    if (!staticMethodName.empty()) {
        smdGetMinecraft = getMethodID(staticMethodName.c_str());
    }
    
    std::string playerFieldName = Mapping::getFieldName("net/minecraft/client/Minecraft", "player");
    if (!playerFieldName.empty()) {
        fdPlayer = getFieldID(playerFieldName.c_str());
    }
    
    std::string worldFieldName = Mapping::getFieldName("net/minecraft/client/Minecraft", "world");
    if (!worldFieldName.empty()) {
        fdWorld = getFieldID(worldFieldName.c_str());
    }
    
    std::string objectMouseOverFieldName = Mapping::getFieldName("net/minecraft/client/Minecraft", "objectMouseOver");
    if (!objectMouseOverFieldName.empty()) {
        fdObjectMouseOver = getFieldID(objectMouseOverFieldName.c_str());
    }
    
    std::string gameSettingsFieldName = Mapping::getFieldName("net/minecraft/client/Minecraft", "gameSettings");
    if (!gameSettingsFieldName.empty()) {
        fdGameSettings = getFieldID(gameSettingsFieldName.c_str());
    }
    
    std::string timerFieldName = Mapping::getFieldName("net/minecraft/client/Minecraft", "timer");
    if (!timerFieldName.empty()) {
        fdTimer = getFieldID(timerFieldName.c_str());
    }
    
    // Continue for other fields...
    
    // init reflection backup
    reflectiveObjectMouseOverField = nullptr;
    useReflectionForObjectMouseOver = false;

    // If the direct field lookup failed, try reflection
    if (fdObjectMouseOver == nullptr) {
        JNIEnv *env = phantom->getEnv();
        if (env) {
            // Use the mapped class name for reflection
            std::string mappedClassName = mcClass->name;
            std::replace(mappedClassName.begin(), mappedClassName.end(), '/', '.');
            
            jclass javaClass = env->FindClass(mappedClassName.c_str());
            if (javaClass) {
                findObjectMouseOverFieldViaReflection(env, javaClass);
                env->DeleteLocalRef(javaClass);
            }
        }
    }
}
