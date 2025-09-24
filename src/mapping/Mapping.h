#pragma once

#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include "CM.h"

extern std::mutex lookup_mutex;  // declare
enum GameVersions {
    CASUAL_1_7_10,
    CASUAL_1_8,
    FORGE_1_7_10,
    FORGE_1_8,
    FEATHER_1_8,
    LUNAR_1_7_10,
    LUNAR_1_8
};

extern GameVersions g_GameVersion;
enum class MappingType {
    Classic = 1,
    ClassType,
    MethodType
};

// Global lookup map and mutex
extern std::map<std::string, std::shared_ptr<CM>> lookup;
extern std::mutex lookup_mutex;

class Mapping {
public:
    // Initialization
    static void Initialize(GameVersions version);

    // Classic mapping retrieval
   static std::string		Get(const char* mapping, int type = 1);

    // CM class access
    static CM* getClass(const char* key);
    static const char* getClassName(const char* key);


};
