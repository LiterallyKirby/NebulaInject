#pragma once
#include <jni.h>
#include <iostream>
#include <string>

struct AxisAlignedBB_t {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

class AxisAlignedBB {
private:
    JavaVM* m_jvm;
    jobject m_globalObj;

    JNIEnv* AttachIfNeeded(bool& attached) const;
    void DetachIfNeeded(bool attached) const;

public:
    AxisAlignedBB(JNIEnv* env, jobject aabbLocal);
    AxisAlignedBB(AxisAlignedBB&& other) noexcept;
    AxisAlignedBB& operator=(AxisAlignedBB&& other) noexcept;
    ~AxisAlignedBB();

    AxisAlignedBB_t GetNativeBoundingBox() const;
    void SetNativeBoundingBox(const AxisAlignedBB_t& buffer) const;
};
