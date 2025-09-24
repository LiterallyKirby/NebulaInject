
#include "ModuleTemplate.h"
#include <iostream>
#include <Minecraft.h>
#include <imgui.h>


ModuleTemplate::ModuleTemplate(Phantom* phantom)
    : Cheat("ModuleTemplate", "A template module for cheats"), phantom(phantom) {


    enabled = false;
    exampleSetting = 10;

    initialize();
}

ModuleTemplate::~ModuleTemplate() {
    cleanup();
}

void ModuleTemplate::initialize() {
    // Initialization logic (register timers, hooks, etc.)
    std::cout << "[ModuleTemplate] Initialized" << std::endl;
}

void ModuleTemplate::cleanup() {
    // Cleanup logic (stop threads, release resources, etc.)
    std::cout << "[ModuleTemplate] Cleaned up" << std::endl;
}


void ModuleTemplate::run(Minecraft* mc) {
    if (!mc) return;
    if (!enabled) return;
    if (!phantom) return;

    std::cout << "[ModuleTemplate] Running with setting: " << exampleSetting << std::endl;

    JNIEnv* env = phantom->getEnv();
    if (!env) return;

    jobject player = Minecraft::GetThePlayer(env); // ✅ now env is defined
    if (player) {
        // do something with the player
    }
}


void ModuleTemplate::renderSettings() {
    ImGui::Text("ModuleTemplate Settings");
    ImGui::Separator();

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::SliderInt("Example Setting", &exampleSetting, 0, 100);
}
