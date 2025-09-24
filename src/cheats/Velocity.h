
#pragma once

#include "Cheat.h" // <-- THIS IS THE FIX
#include <Minecraft.h>
#include <string>
#include "Cheat.h"


enum class VelocityMode {
    Clamp,
    Scale,
    Override
};

class VelocityModule : public Cheat {
private:
    VelocityMode mode; // Mode setting
public:
    bool enabled;
    bool airOnly;
    bool movingOnly;
    bool weaponOnly;
    int chance;
    int delay;
    float horizontal;
    float vertical;

    VelocityModule();
    ~VelocityModule();

    void initialize();
    void cleanup();

    void clamp();
    void scale(float h, float v);
    void overrideMode(float h, float v);

    void run(Minecraft* mc);
    void renderSettings();
};


