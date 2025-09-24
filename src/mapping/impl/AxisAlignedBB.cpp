#include "AxisAlignedBB.h"
#include "../Mapping.h" // mapping helper your project uses
#include <iostream>

AxisAlignedBB::AxisAlignedBB(JNIEnv* env, jobject aabbLocal)
    : m_jvm(nullptr), m_globalObj(nullptr)
{
    if (!env || !aabbLocal) return;

    // store JavaVM*
    if (env->GetJavaVM(&m_jvm) != JNI_OK) {
        m_jvm = nullptr;
        std::cerr << "[AxisAlignedBB] Failed to get JavaVM\n";
    }

    // create a global ref to hold onto the object safely across threads/calls
    m_globalObj = env->NewGlobalRef(aabbLocal);
    if (!m_globalObj) {
        std::cerr << "[AxisAlignedBB] Failed to create global ref for AxisAlignedBB object\n";
    }
}

AxisAlignedBB::AxisAlignedBB(AxisAlignedBB&& other) noexcept
    : m_jvm(other.m_jvm), m_globalObj(other.m_globalObj)
{
    other.m_jvm = nullptr;
    other.m_globalObj = nullptr;
}

AxisAlignedBB& AxisAlignedBB::operator=(AxisAlignedBB&& other) noexcept {
    if (this == &other) return *this;
    // release our existing global ref safely
    if (m_globalObj && m_jvm) {
        bool attached = false;
        JNIEnv* env = AttachIfNeeded(m_jvm, attached);
        if (env) env->DeleteGlobalRef(m_globalObj);
        DetachIfNeeded(m_jvm, attached);
    }
    m_jvm = other.m_jvm;
    m_globalObj = other.m_globalObj;
    other.m_jvm = nullptr;
    other.m_globalObj = nullptr;
    return *this;
}

AxisAlignedBB::~AxisAlignedBB() {
    if (!m_globalObj) return;
    if (!m_jvm) {
        // Can't safely delete global ref without JavaVM. Leak is safer than crash,
        // but print a warning.
        std::cerr << "[AxisAlignedBB] Warning: JavaVM* missing in destructor; leaking global ref\n";
        return;
    }

    bool attached = false;
    JNIEnv* env = AttachIfNeeded(m_jvm, attached);
    if (env) {
        env->DeleteGlobalRef(m_globalObj);
    } else {
        std::cerr << "[AxisAlignedBB] Warning: failed to get JNIEnv to delete global ref\n";
    }
    DetachIfNeeded(m_jvm, attached);

    m_globalObj = nullptr;
}

// Attach current thread (if necessary) and return JNIEnv*.
// Sets attached=true if the function attached the thread and caller should detach later.
JNIEnv* AxisAlignedBB::AttachIfNeeded(JavaVM* jvm, bool& attached) {
    attached = false;
    if (!jvm) return nullptr;
    JNIEnv* env = nullptr;
    jint res = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    if (res == JNI_OK) {
        return env;
    } else if (res == JNI_EDETACHED) {
        // attach
#if defined(__ANDROID__) || defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
        if (jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) == JNI_OK) {
            attached = true;
            return env;
        }
#endif
        return nullptr;
    } else {
        return nullptr;
    }
}

void AxisAlignedBB::DetachIfNeeded(JavaVM* jvm, bool attached) {
    if (!jvm || !attached) return;
#if defined(__ANDROID__) || defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
    jvm->DetachCurrentThread();
#endif
}

AxisAlignedBB_t AxisAlignedBB::GetNativeBoundingBox(JNIEnv* env) const {
    AxisAlignedBB_t out{};
    if (!env || !m_globalObj) return out;

    // Get the class of the AABB object
    jclass aabbClazz = env->GetObjectClass(m_globalObj);
    if (!aabbClazz) return out;

    // Use mapping to resolve obf field names
    const std::string minXName = Mapping::Get("minX");
    const std::string minYName = Mapping::Get("minY");
    const std::string minZName = Mapping::Get("minZ");
    const std::string maxXName = Mapping::Get("maxX");
    const std::string maxYName = Mapping::Get("maxY");
    const std::string maxZName = Mapping::Get("maxZ");

    jfieldID minXField = env->GetFieldID(aabbClazz, minXName.c_str(), "D");
    jfieldID minYField = env->GetFieldID(aabbClazz, minYName.c_str(), "D");
    jfieldID minZField = env->GetFieldID(aabbClazz, minZName.c_str(), "D");
    jfieldID maxXField = env->GetFieldID(aabbClazz, maxXName.c_str(), "D");
    jfieldID maxYField = env->GetFieldID(aabbClazz, maxYName.c_str(), "D");
    jfieldID maxZField = env->GetFieldID(aabbClazz, maxZName.c_str(), "D");

    if (!minXField || !minYField || !minZField ||
        !maxXField || !maxYField || !maxZField) {
        env->DeleteLocalRef(aabbClazz);
        return out;
    }

    double minXd = env->GetDoubleField(m_globalObj, minXField);
    double minYd = env->GetDoubleField(m_globalObj, minYField);
    double minZd = env->GetDoubleField(m_globalObj, minZField);
    double maxXd = env->GetDoubleField(m_globalObj, maxXField);
    double maxYd = env->GetDoubleField(m_globalObj, maxYField);
    double maxZd = env->GetDoubleField(m_globalObj, maxZField);

    env->DeleteLocalRef(aabbClazz);

    out.minX = static_cast<float>(minXd);
    out.minY = static_cast<float>(minYd);
    out.minZ = static_cast<float>(minZd);
    out.maxX = static_cast<float>(maxXd);
    out.maxY = static_cast<float>(maxYd);
    out.maxZ = static_cast<float>(maxZd);

    return out;
}

void AxisAlignedBB::SetNativeBoundingBox(const AxisAlignedBB_t& buffer, JNIEnv* env) const {
    if (!env || !m_globalObj) return;

    jclass aabbClazz = env->GetObjectClass(m_globalObj);
    if (!aabbClazz) return;

    const std::string minXName = Mapping::Get("minX");
    const std::string minYName = Mapping::Get("minY");
    const std::string minZName = Mapping::Get("minZ");
    const std::string maxXName = Mapping::Get("maxX");
    const std::string maxYName = Mapping::Get("maxY");
    const std::string maxZName = Mapping::Get("maxZ");

    jfieldID minXField = env->GetFieldID(aabbClazz, minXName.c_str(), "D");
    jfieldID minYField = env->GetFieldID(aabbClazz, minYName.c_str(), "D");
    jfieldID minZField = env->GetFieldID(aabbClazz, minZName.c_str(), "D");
    jfieldID maxXField = env->GetFieldID(aabbClazz, maxXName.c_str(), "D");
    jfieldID maxYField = env->GetFieldID(aabbClazz, maxYName.c_str(), "D");
    jfieldID maxZField = env->GetFieldID(aabbClazz, maxZName.c_str(), "D");

    if (!minXField || !minYField || !minZField ||
        !maxXField || !maxYField || !maxZField) {
        env->DeleteLocalRef(aabbClazz);
        return;
    }

    env->SetDoubleField(m_globalObj, minXField, static_cast<jdouble>(buffer.minX));
    env->SetDoubleField(m_globalObj, minYField, static_cast<jdouble>(buffer.minY));
    env->SetDoubleField(m_globalObj, minZField, static_cast<jdouble>(buffer.minZ));
    env->SetDoubleField(m_globalObj, maxXField, static_cast<jdouble>(buffer.maxX));
    env->SetDoubleField(m_globalObj, maxYField, static_cast<jdouble>(buffer.maxY));
    env->SetDoubleField(m_globalObj, maxZField, static_cast<jdouble>(buffer.maxZ));

    env->DeleteLocalRef(aabbClazz);
}
