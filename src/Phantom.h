//
// Created by somepineaple on 1/25/22.
//

#ifndef PHANTOM_PHANTOM_H
#define PHANTOM_PHANTOM_H

#include <jni.h>
#include <jvmti.h>
#include <vector>
#include "mapping/Mapping.h"
class Cheat;

class Phantom {
public:
    Phantom();
 JavaVM *jvm;
    JNIEnv *env;
GameVersions DetectGameVersion(); // Add this
    void runClient();
    void onKey(int key);

    JavaVM *getJvm();
    JNIEnv *getEnv();
    void setRunning(bool p_running);
    bool isRunning() const;

    // Dump Minecraft JNI mappings to a header file
    void dumpMCToHeader(jobject mcObj);

private:
    std::vector<Cheat *> cheats{};

    bool running;
   
};

#endif // PHANTOM_PHANTOM_H
