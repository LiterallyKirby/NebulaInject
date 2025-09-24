//
// Created by somepineaple on 1/30/22.
// Updated for Nebula theme
//

#ifndef NEBULA_NEBULAWINDOW_H
#define NEBULA_NEBULAWINDOW_H

#include <vector>
#include <GL/glew.h>  
#include <SDL.h>
#include "../cheats/Cheat.h"

class NebulaWindow {
public:
    NebulaWindow(int width, int height, const char *title);
    void setup();
    void destruct();
    void update(const std::vector<Cheat*>& cheats, bool &running, bool inGame);

private:
    int width, height;
    const char *title;
    int style;

    SDL_Window *window;
    SDL_GLContext glContext;
};

#endif //NEBULA_NEBULAWINDOW_H
