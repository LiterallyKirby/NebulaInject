#pragma once

#include <random>
#include "Cheat.h"

enum class VelocityMode {
    Clamp = 0,
    Scale = 1,
    Override = 2
};

class VelocityModule : public Cheat {
public:
    VelocityModule();
    ~VelocityModule();

    void run(Minecraft* mc) override;
    void renderSettings() override;

    // Configuration methods
    void clamp();
    void scale(float h, float v);
    void overrideMode(float h, float v);

private:
    void initialize();
    void cleanup();

    // Settings
    VelocityMode mode;
    bool airOnly;
    bool movingOnly;
    bool weaponOnly;
    int chance;
    int delay;
    float horizontal;
    float vertical;
    
    // Random number generation for chance calculation
    mutable std::mt19937 rng;
};
