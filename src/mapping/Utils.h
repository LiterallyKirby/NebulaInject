#pragma once
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include <vector>






Vec4 Multiply(const Vec4& vec, const std::vector<float>& mat);
bool WorldToScreen(Vec3 worldPos, Vec2& screen, std::vector<float>& modelView, std::vector<float>& projection, int width, int height);

Vec3 FromHSB(float h, float s, float v);
int RandomInteger(const int min, const int max);

