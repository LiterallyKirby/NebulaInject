#pragma once
#include "cmath"
struct Vec3 
{
	float x, y, z;

	float distance(const Vec3& other) const
	{
		return sqrtf(powf(x - other.x, 2.0) + powf(y - other.y, 2.0) + powf(z - other.z, 2.0));
	}
};

struct Vec3D 
{
	double x, y, z;

	double distance(const Vec3D& other) const 
	{
		return sqrt(pow(x - other.x, 2.0) + pow(y - other.y, 2.0) + pow(z - other.z, 2.0));
	}
};
