#include "AxisAlignedBB.h"
#include "../Mapping.h"

AxisAlignedBB::AxisAlignedBB(JNIEnv* env, jobject aabbLocal)
    : m_jvm(nullptr), m_globalObj(nullptr)
{
    if (!env || !aabbLocal) return;

    if (env->GetJavaVM(&m_jvm) != JNI_OK) {
        std::cerr << "[AxisAlignedBB] Failed to get JavaVM\n";
        return;
    }

    m_globalObj = env->NewGlobalRef(aabbLocal);
    if (!m_globalObj) {
        std::cerr << "[AxisAlignedBB] Failed to create global ref for AxisAlignedBB\n";
    }
}

AxisAlignedBB::AxisAlignedBB(AxisAlignedBB&& other) noexcept
    : m_jvm(other.m_jvm), m_globalObj(other.m_globalObj)
{
    other.m_jvm = nullptr;
    other.m_globalObj = nullptr;
}

AxisAlignedBB& AxisAlignedBB::operator=(AxisAlignedBB&& other) noexcept {
    if (this != &other) {
        bool attached = false;
        JNIEnv* env = AttachIfNeeded(attached);
        if (env && m_globalObj) {
            env->DeleteGlobalRef(m_globalObj);
        }
        DetachIfNeeded(attached);

        m_jvm = other.m_jvm;
        m_globalObj = other.m_globalObj;

        other.m_jvm = nullptr;
        other.m_globalObj = nullptr;
    }
    return *this;
}

AxisAlignedBB::~AxisAlignedBB() {
    if (!m_globalObj) return;

    if (!m_jvm) {
        std::cerr << "[AxisAlignedBB] JavaVM* missing; leaking global ref\n";
        return;
    }

    bool attached = false;
    JNIEnv* env = AttachIfNeeded(attached);
    if (!env) {
        std::cerr << "[AxisAlignedBB] Failed to attach thread for cleanup\n";
        return;
    }

    env->DeleteGlobalRef(m_globalObj);
    m_globalObj = nullptr;

    DetachIfNeeded(attached);
}

JNIEnv* AxisAlignedBB::AttachIfNeeded(bool& attached) const {
    attached = false;
    if (!m_jvm) return nullptr;

    JNIEnv* env = nullptr;
    jint res = m_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    if (res == JNI_OK) {
        return env;
    }
    if (res == JNI_EDETACHED) {
#if defined(__ANDROID__) || defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
        if (m_jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) == JNI_OK) {
            attached = true;
            return env;
        }
#endif
        std::cerr << "[AxisAlignedBB] Failed to attach thread\n";
    }
    return nullptr;
}

void AxisAlignedBB::DetachIfNeeded(bool attached) const {
    if (m_jvm && attached) {
#if defined(__ANDROID__) || defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
        m_jvm->DetachCurrentThread();
#endif
    }
}

AxisAlignedBB_t AxisAlignedBB::GetNativeBoundingBox() const {
    AxisAlignedBB_t out{};
    bool attached = false;
    JNIEnv* env = AttachIfNeeded(attached);
    if (!env || !m_globalObj) return out;

    jclass aabbClazz = env->GetObjectClass(m_globalObj);
    if (!aabbClazz) {
        DetachIfNeeded(attached);
        return out;
    }

    auto getFieldSafe = [&](const char* name) -> jfieldID {
        jfieldID field = env->GetFieldID(aabbClazz, name, "D");
        if (!field) {
            std::cerr << "[AxisAlignedBB] Failed to find field: " << name << "\n";
        }
        return field;
    };

    double minX = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("minX").c_str()));
    double minY = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("minY").c_str()));
    double minZ = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("minZ").c_str()));
    double maxX = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("maxX").c_str()));
    double maxY = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("maxY").c_str()));
    double maxZ = env->GetDoubleField(m_globalObj, getFieldSafe(Mapping::Get("maxZ").c_str()));

    env->DeleteLocalRef(aabbClazz);
    DetachIfNeeded(attached);

    return { static_cast<float>(minX), static_cast<float>(minY), static_cast<float>(minZ),
             static_cast<float>(maxX), static_cast<float>(maxY), static_cast<float>(maxZ) };
}
