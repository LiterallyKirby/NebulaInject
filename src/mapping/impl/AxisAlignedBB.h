#pragma once
#include <jni.h>
struct AxisAlignedBB_t {
	float minX, minY, minZ, maxX, maxY, maxZ;
};

class AxisAlignedBB
{
public:
	AxisAlignedBB_t GetNativeBoundingBox(JNIEnv* env);
	void SetNativeBoundingBox(AxisAlignedBB_t buffer, JNIEnv* env);
};
