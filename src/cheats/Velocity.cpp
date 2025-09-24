#include "Velocity.h"

#include <Minecraft.h>
#include <imgui.h>

#include <iostream>

#include "Cheat.h"  // Add this before defining VelocityModule
#include "EntityPlayerSP.h"
#include "ItemStack.h"

VelocityModule::VelocityModule()
    : Cheat("Velocity", "Velocity control module"), mode(VelocityMode::Clamp) {
    airOnly = false;
    movingOnly = false;
    weaponOnly = false;
    chance = 100;
    delay = 0;
    horizontal = 100.0f;
    vertical = 100.0f;

    initialize();
}

VelocityModule::~VelocityModule() { cleanup(); }

void VelocityModule::initialize() {
    std::cout << "[VelocityModule] Initialized" << std::endl;
}

void VelocityModule::cleanup() {
    std::cout << "[VelocityModule] Cleaned up" << std::endl;
}

void VelocityModule::clamp() {
    if (horizontal < 0.0f) horizontal = 0.0f;
    if (horizontal > 200.0f) horizontal = 200.0f;
    if (vertical < 0.0f) vertical = 0.0f;
    if (vertical > 200.0f) vertical = 200.0f;
}

void VelocityModule::scale(float h, float v) {
    horizontal *= h;
    vertical *= v;
    clamp();
}

void VelocityModule::overrideMode(float h, float v) {
    horizontal = h;
    vertical = v;
    clamp();
}

void VelocityModule::run(Minecraft* mc) {
    if (!mc) return;

    auto env = mc->env;
    jobject playerObj = Minecraft::GetThePlayer(env);
    if (!playerObj) return;

    Player thePlayer(env, playerObj);

    // Apply your existing condition checks
    const auto myPrevPos = thePlayer.GetPreviousPos();
    const auto myPos = thePlayer.GetPos();

    if (airOnly && thePlayer.IsOnGround()) return;
    if (movingOnly && myPrevPos.x == myPos.x && myPrevPos.y == myPos.y && myPrevPos.z == myPos.z) return;
    if (weaponOnly) {
        const auto heldItem = (ItemStack*)thePlayer.GetHeldItem();
        if (!heldItem || !heldItem->IsWeapon()) return;
    }

    // Use the original timing logic - trigger on first frame of being hurt
    const int maxHurtTime = thePlayer.GetMaxHurtResistantTime();
    const int hurtTime = thePlayer.GetHurtResistantTime();
    
if (hurtTime > 0 && hurtTime >= (maxHurtTime * 0.9)) {
        // FIXED: Apply chance check correctly
        // Generate random number 1-100, if it's greater than chance, skip
        if (chance < 100 && (rand() % 100 + 1) > chance) return;
        
        // Apply velocity modification immediately
        switch (mode) {
            case VelocityMode::Scale:
                thePlayer.SetMotionX(thePlayer.GetMotionX() * (horizontal / 100.0));
                thePlayer.SetMotionY(thePlayer.GetMotionY() * (vertical / 100.0));
                thePlayer.SetMotionZ(thePlayer.GetMotionZ() * (horizontal / 100.0));
                break;
            case VelocityMode::Override:
                thePlayer.SetMotionX(horizontal / 100.0);
                thePlayer.SetMotionY(vertical / 100.0);
                thePlayer.SetMotionZ(horizontal / 100.0);
                break;
            case VelocityMode::Clamp:
                // Apply clamping logic
                double motionX = thePlayer.GetMotionX();
                double motionY = thePlayer.GetMotionY();
                double motionZ = thePlayer.GetMotionZ();
                
                double maxH = horizontal / 100.0;
                double maxV = vertical / 100.0;
                
                if (motionX > maxH) thePlayer.SetMotionX(maxH);
                else if (motionX < -maxH) thePlayer.SetMotionX(-maxH);
                
                if (motionY > maxV) thePlayer.SetMotionY(maxV);
                else if (motionY < -maxV) thePlayer.SetMotionY(-maxV);
                
                if (motionZ > maxH) thePlayer.SetMotionZ(maxH);
                else if (motionZ < -maxH) thePlayer.SetMotionZ(-maxH);
                break;
        }
    }
}

void VelocityModule::renderSettings() {
    ImGui::Text("Velocity Module Settings");
    ImGui::Separator();

    ImGui::Checkbox("Air Only", &airOnly);
    ImGui::Checkbox("Moving Only", &movingOnly);
    ImGui::Checkbox("Weapon Only", &weaponOnly);

    // Mode selector with descriptions
    const char* modeItems[] = { "Clamp", "Scale", "Override" };
    int currentMode = static_cast<int>(mode);

    if (ImGui::Combo("Mode", &currentMode, modeItems, IM_ARRAYSIZE(modeItems))) {
        mode = static_cast<VelocityMode>(currentMode);
    }

    ImGui::TextWrapped("Mode Description:");
    switch (mode) {
        case VelocityMode::Clamp:
            ImGui::TextWrapped("Clamps velocity factors between min and max values.");
            break;
        case VelocityMode::Scale:
            ImGui::TextWrapped("Scales the current velocity by horizontal/vertical factors.");
            break;
        case VelocityMode::Override:
            ImGui::TextWrapped("Overrides velocity with horizontal/vertical factors.");
            break;
    }

    ImGui::SliderInt("Chance (%)", &chance, 0, 100);
    ImGui::SliderInt("Delay", &delay, 0, 20);
    ImGui::SliderFloat("Horizontal Factor (%)", &horizontal, 0.0f, 200.0f, "%.1f");
    ImGui::SliderFloat("Vertical Factor (%)", &vertical, 0.0f, 200.0f, "%.1f");
}
