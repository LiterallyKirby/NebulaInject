
#pragma once

#include "Cheat.h"       // base cheat class
#include <memory>

class ModuleTemplate : public Cheat {
public:
   ModuleTemplate(Phantom* phantom);
    ~ModuleTemplate();

    void run(class Minecraft* mc) override; // main loop
    void renderSettings() override;         // settings menu

private:
    void initialize();
    void cleanup();
Phantom* phantom; // store phantom pointer

    int exampleSetting; // example user-configurable setting
};
