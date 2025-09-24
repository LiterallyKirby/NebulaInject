#pragma once
#include <jni.h>
#include "../Vec3.h"
class RenderManager
{
public:
	jobject getEntityRender(jobject argEntity, JNIEnv* env);
	Vec3D GetRenderPos(JNIEnv* env);
	Vec3D GetViewerPos(JNIEnv* env);
	bool DoRenderEntity(jobject entity, double x, double y, double z, float entityYaw, float partialTicks, bool p_147939_10_, JNIEnv* env);
};
