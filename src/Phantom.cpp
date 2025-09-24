//
// Created by somepineaple on 1/25/22.
//
#include "Phantom.h"

#include <EntityPlayerSP.h>
#include <Minecraft.h>
#include <World.h>

#include <fstream>
#include <string>
#include <thread>

#include "mapping/Mapping.h"
/* #include <net/minecraft/client/multiplayer/PlayerControllerMP.h> */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "cheats/AutoClicker.h"
#include "cheats/Fastplace.h"
#include "cheats/Reach.h"
#include "cheats/Velocity.h"
#include "ui/KeyManager.h"
#include "ui/PhantomWindow.h"

std::string GetMinecraftWindowTitle() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return "";

    Window root = DefaultRootWindow(display);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char *prop = nullptr;

    Atom netWmName = XInternAtom(display, "_NET_WM_NAME", True);
    Atom utf8String = XInternAtom(display, "UTF8_STRING", True);

    std::string title;

    Window parent, *children;
    unsigned int nChildren;
    if (XQueryTree(display, root, &root, &parent, &children, &nChildren)) {
        for (unsigned int i = 0; i < nChildren; ++i) {
            if (XGetWindowProperty(display, children[i], netWmName, 0, (~0L),
                                   False, utf8String, &actualType,
                                   &actualFormat, &nItems, &bytesAfter,
                                   &prop) == Success) {
                if (prop) {
                    title = std::string(reinterpret_cast<char *>(prop));
                    XFree(prop);
                    break;
                }
            }
        }
        if (children) XFree(children);
    }

    XCloseDisplay(display);
    return title;
}

std::string GameVersionToString(GameVersions version) {
    switch (version) {
        case LUNAR_1_7_10:
            return "LUNAR_1_7_10";
        case LUNAR_1_8:
            return "LUNAR_1_8";
        case FORGE_1_7_10:
            return "FORGE_1_7_10";
        case FORGE_1_8:
            return "FORGE_1_8";
        case CASUAL_1_7_10:
            return "CASUAL_1_7_10";
        case CASUAL_1_8:
            return "CASUAL_1_8";
        case FEATHER_1_8:
            return "FEATHER_1_8";
        default:
            return "UNKNOWN";
    }
}

GameVersions Phantom::DetectGameVersion() {
    std::string title = GetMinecraftWindowTitle();
    std::cout << "Title: " << title << std::endl;

    bool is1_7 = title.find("1.7.10") != std::string::npos;
    if (title.find("Lunar Client") != std::string::npos) {
        std::cout << "Detected Lunar Client" << std::endl;
        return is1_7 ? LUNAR_1_7_10 : LUNAR_1_8;
    }

    bool vanillaMappings = title.find("Badlion Minecraft") != std::string::npos;
    std::cout << "vanillaMappings: " << vanillaMappings << std::endl;

    CM *cm = Mapping::getClass("net/minecraft/launchwrapper/LaunchClassLoader");
    std::cout << "cm: " << (cm ? "FOUND" : "NOT FOUND") << std::endl;
    if (!cm) return CASUAL_1_8;

    CM *cm2 = Mapping::getClass("net/minecraft/launchwrapper/Launch");
    std::cout << "cm2: " << (cm2 ? "FOUND" : "NOT FOUND") << std::endl;
    if (!cm2) return CASUAL_1_8;

    jclass launchClazz = env->FindClass(cm2->name);
    std::cout << "launchClazz: " << (launchClazz ? "FOUND" : "NOT FOUND")
              << std::endl;

    if (cm && launchClazz && !vanillaMappings) {
        std::cout << "Detected Forge" << std::endl;
        return is1_7 ? FORGE_1_7_10 : FORGE_1_8;
    }

    if (Mapping::getClass("net/digitalingot/featheropt/FeatherCoreMod")) {
        std::cout << "Detected Feather Mod" << std::endl;
        return FEATHER_1_8;
    }

    std::cout << "Fallback to casual" << std::endl;
    return is1_7 ? CASUAL_1_7_10 : CASUAL_1_8;
}

GameVersions g_GameVersion = CASUAL_1_7_10;

Phantom::Phantom() {
    running = false;
    jvm = nullptr;
    env = nullptr;

    jsize count;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0) {
        std::cerr << "Failed to get JVM" << std::endl;
        return;
    }

    jint res = jvm->GetEnv((void **)&env, JNI_VERSION_1_8);
    if (res == JNI_EDETACHED)
        res = jvm->AttachCurrentThread((void **)&env, nullptr);
    if (res != JNI_OK) {
        std::cerr << "Failed to attach to JVM: " << res << std::endl;
        return;
    }

    Mapping::Initialize(CASUAL_1_8);  // load some default mappings first
    g_GameVersion = DetectGameVersion();
    Mapping::Initialize(g_GameVersion);

    cheats.push_back(new AutoClicker());
    cheats.push_back(new FastPlaceModule(this));
    cheats.push_back(new VelocityModule());
    cheats.push_back(new ReachModule(this));

    std::cout << "Phantom initialized successfully" << std::endl;
}

void Phantom::runClient() {
    running = true;
    std::cout << "Starting client..." << std::endl;

    auto *mc = new Minecraft(this);
    auto *window = new NebulaWindow(700, 500, "Phantom");
    window->setup();
    auto *keyManager = new KeyManager();

    int loopCount = 0;
    while (running) {
        try {
            // Check JNI environment is still valid
            if (!env || !jvm) {
                std::cerr << "JNI environment became invalid" << std::endl;
                break;
            }

            // Clear any pending JNI exceptions
            if (env->ExceptionCheck()) {
                std::cerr << "Clearing pending JNI exception" << std::endl;
                env->ExceptionDescribe();
                env->ExceptionClear();
            }

            // Get player object
            jobject playerObj = mc->GetThePlayer(env);
            bool hasPlayer = (playerObj != nullptr);

            // Get world object
            jobject worldObj = mc->GetTheWorld(env);
            bool hasWorld = (worldObj != nullptr);

            bool inGame = hasPlayer && hasWorld;

            // DEBUG OUTPUT
            if (loopCount % 100 == 0) {
                std::cout << "DEBUG - Loop: " << loopCount << std::endl;
                std::cout << "  Player object: "
                          << (hasPlayer ? "EXISTS" : "NULL") << std::endl;
                std::cout << "  World object: "
                          << (hasWorld ? "EXISTS" : "NULL") << std::endl;
                std::cout << "  InGame status: " << (inGame ? "TRUE" : "FALSE")
                          << std::endl;
                std::cout << "  Cheats count: " << cheats.size() << std::endl;
                std::cout << "Detected game version: "
                          << GameVersionToString(g_GameVersion) << std::endl;

                std::cout << "Axe class: "
                          << Mapping::Get("net/minecraft/item/ItemAxe") << "\n";
                std::cout << "Sword class: "
                          << Mapping::Get("net/minecraft/item/ItemSword")
                          << "\n";
            }

            if (!inGame) {
                // Pass false to indicate not in game
                window->update(cheats, running, false);

                // COMMENT OUT the DeleteLocalRef calls for now
                /*
                if (playerObj) {
                    env->DeleteLocalRef(playerObj);
                    playerObj = nullptr;
                }
                if (worldObj) {
                    env->DeleteLocalRef(worldObj);
                    worldObj = nullptr;
                }
                */

                loopCount++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Handle key updates in main thread (safer than detached thread)
            keyManager->updateKeys(this);

            // Run enabled cheats/modules
            for (Cheat *cheat : cheats) {
                try {
                    if (cheat->enabled) {
                        cheat->run(mc);
                    } else {
                        cheat->reset(mc);
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Exception in cheat execution: " << e.what()
                              << std::endl;
                }
            }

            // Update UI with inGame = true
            window->update(cheats, running, true);

            // COMMENT OUT the DeleteLocalRef calls for now
            /*
            if (playerObj) {
                env->DeleteLocalRef(playerObj);
                playerObj = nullptr;
            }
            if (worldObj) {
                env->DeleteLocalRef(worldObj);
                worldObj = nullptr;
            }
            */

            loopCount++;

            // Small delay to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        } catch (const std::exception &e) {
            std::cerr << "Exception in main loop: " << e.what() << std::endl;
            // Continue running despite exceptions
        }
    }

    std::cout << "Stopping client..." << std::endl;

    // Ensure that all JNI operations are done before detaching
    window->destruct();

    // COMMENT OUT detachment for now
    /*
    if (jvm) {
        jvm->DetachCurrentThread();
    }
    */

    // Clean up
    //  delete mc;
    //  delete window;
    //  delete keyManager;

    std::cout << "Client stopped." << std::endl;
}

void Phantom::onKey(int key) {
    for (Cheat *cheat : cheats) {
        if (cheat->binding) {
            cheat->bind = key;
            cheat->binding = false;
            return;
        }
    }

    for (Cheat *cheat : cheats) {
        if (cheat->bind == key) {
            cheat->enabled = !cheat->enabled;
            std::cout << "Toggled cheat: " << cheat->name << " -> "
                      << (cheat->enabled ? "ON" : "OFF") << std::endl;
        }
    }
}

JavaVM *Phantom::getJvm() { return jvm; }

JNIEnv *Phantom::getEnv() { return env; }

void Phantom::setRunning(bool p_running) { this->running = p_running; }

bool Phantom::isRunning() const { return running; }
