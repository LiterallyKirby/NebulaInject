#ifndef PHANTOM_MAPPING_H
#define PHANTOM_MAPPING_H

#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include "CM.h"

enum GameVersions {
    CASUAL_1_7_10,
    CASUAL_1_8,
    FORGE_1_7_10,
    FORGE_1_8,
    FEATHER_1_8,
    LUNAR_1_7_10,
    LUNAR_1_8
};

// Global lookup map and mutex
extern std::map<std::string, std::shared_ptr<CM>> lookup;
extern std::mutex lookup_mutex;

class Mapping {
public:
    // Existing functions
    static void Initialize(const GameVersions version);
    static std::string Get(const char* mapping, int type = 1);
    static CM* getClass(const char* key);
    static const char* getClassName(const char* key);
    
    // New CM-based helper functions
    static std::string getFieldName(const char* className, const char* fieldName);
    static std::string getMethodName(const char* className, const char* methodName);
    static std::string getFieldDesc(const char* className, const char* fieldName);
    static std::string getMethodDesc(const char* className, const char* methodName);
};

#endif //PHANTOM_MAPPING_H
