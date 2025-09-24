#pragma once

#include <jni.h>

struct AxisAlignedBB_t {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

class AxisAlignedBB {
public:
    // Construct from a local jobject (as returned by GetObjectField).
    // Will create and own a global ref to the Java object.
    AxisAlignedBB(JNIEnv* env, jobject aabbLocal);

    // Moveable but not copyable.
    AxisAlignedBB(AxisAlignedBB&&) noexcept;
    AxisAlignedBB& operator=(AxisAlignedBB&&) noexcept;
    AxisAlignedBB(const AxisAlignedBB&) = delete;
    AxisAlignedBB& operator=(const AxisAlignedBB&) = delete;

    // Destructor deletes the global ref. Uses JavaVM to attach if needed.
    ~AxisAlignedBB();

    // Read double fields from the Java AABB object and return as floats.
    AxisAlignedBB_t GetNativeBoundingBox(JNIEnv* env) const;

    // Write float fields into the Java AABB object (stored as doubles).
    void SetNativeBoundingBox(const AxisAlignedBB_t& buffer, JNIEnv* env) const;

    // Returns the global jobject (useful for passing around).
    jobject GetGlobalObject() const { return m_globalObj; }

    // Whether wrapper successfully holds a global ref.
    bool IsValid() const { return m_globalObj != nullptr; }

private:
    JavaVM* m_jvm = nullptr;   // not owned
    jobject m_globalObj = nullptr; // global ref to Java AABB object (or nullptr)

    // Helper: attach current thread and return JNIEnv*; sets attached=true if we attached.
    static JNIEnv* AttachIfNeeded(JavaVM* jvm, bool& attached);

    // Helper: detach when attached
    static void DetachIfNeeded(JavaVM* jvm, bool attached);
};
