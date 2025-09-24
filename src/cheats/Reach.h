#pragma once
#include "Cheat.h"
#include <memory>

class ReachModule : public Cheat {
public:
    ReachModule(Phantom* phantom);
    ~ReachModule();
    
    void run(class Minecraft* mc) override;
    void renderSettings() override;
    
private:
    void initialize();
    void cleanup();
    
    Phantom* phantom;
    
    // Settings
    bool enabled;
    float distance;
    float hitboxExpansion;
    bool onlyWeapon;
    bool onGround;
    bool liquidCheck;
    bool comboMode;
    
    // Combo tracking variables
    unsigned long long lastLocalHit;
    unsigned long long lastTargetHit;
    unsigned long long firstTargetHit;
    int targetHitCount;
    bool inCombo;
};
