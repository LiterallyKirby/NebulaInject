#pragma once
#include "jni.h"
class Timer
{
public:
	float GetRenderPartialTicks(JNIEnv* env);
	float GetTimerSpeed(JNIEnv* env);
	void SetTimerSpeed(float v, JNIEnv* env);
};
