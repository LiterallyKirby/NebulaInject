#pragma once
#include "Cheat.h" // base cheat class
#include <memory>
#include <atomic>
#include "../utils/MSTimer.h"
#include <X11/Xlib.h>

class FastPlaceModule : public Cheat {
public:
    FastPlaceModule(Phantom* phantom);
    ~FastPlaceModule();
    
    void run(class Minecraft* mc) override; // main loop
    void renderSettings() override; // settings menu

private:
    void initialize();
    void cleanup();
    void performRightClick();
    void updateValues();
    
    Phantom* phantom; // store phantom pointer
    
    // Basic settings
    bool blockOnly; // only fast place with blocks
    
    // Timing settings
    float cps;
    bool randomizeCps;
    float cpsVariance;
    float jitterMs;
    float holdLength;
    float holdLengthRandom;
    
    // Variation settings
    float dropChance;
    float spikeChance;  
    float spikeMultiplier;
    
    // Timing state
    std::unique_ptr<MSTimer> clickTimer;
    std::unique_ptr<MSTimer> eventTimer;
    int nextDelay;
    int eventDelay;
    int nextEventDelay;
    
    // State tracking
    std::atomic<bool> isHolding;
    std::atomic<bool> shouldClick;
    
    // X11 display for mouse input
    Display* xDisplay;
};
