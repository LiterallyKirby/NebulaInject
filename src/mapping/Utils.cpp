
#include "Utils.h"

#include <GL/gl.h>
#include <random>





Vec4 Multiply(const Vec4& vec, const std::vector<float>& mat)
{
	if (mat.size() != 16)
		return Vec4();

	return Vec4(
		vec.x * mat[0] + vec.y * mat[4] + vec.z * mat[8] + vec.w * mat[12],
		vec.x * mat[1] + vec.y * mat[5] + vec.z * mat[9] + vec.w * mat[13],
		vec.x * mat[2] + vec.y * mat[6] + vec.z * mat[10] + vec.w * mat[14],
		vec.x * mat[3] + vec.y * mat[7] + vec.z * mat[11] + vec.w * mat[15]
	);
}

bool WorldToScreen(Vec3 worldPos, Vec2& screen, std::vector<float>& modelView, std::vector<float>& projection, int width, int height)
{
	if (projection.size() != 16 || modelView.size() != 16) {
		return false;
	}
	
	const auto clipSpacePos = Multiply(Multiply(Vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f), modelView), projection);
	const auto ndcSpacePos = Vec3(clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);

	if (ndcSpacePos.z < -1.0 || ndcSpacePos.z > 1.0)
	{
		return false;
	}

	screen.x = width * ((ndcSpacePos.x + 1.0f) / 2.0f);
	screen.y = height * ((1.0f - ndcSpacePos.y) / 2.0f);
	return true;
}

Vec3 FromHSB(float h, float s, float v)
{
	float _r, _g, _b;

	float _h = h == 1.0f ? 0 : h * 6.0f;
	auto i = static_cast<int>(trunc(_h));
	float f = _h - i;

	const float p = v * (1.f - s);
	const float q = v * (1.f - s * f);
	const float t = v * (1.f - s * (1.f - f));

	switch (i) {
	case 0:
		_r = v;
		_g = t;
		_b = p;
		break;

	case 1:
		_r = q;
		_g = v;
		_b = p;
		break;

	case 2:
		_r = p;
		_g = v;
		_b = t;
		break;

	case 3:
		_r = p;
		_g = q;
		_b = v;
		break;

	case 4:
		_r = t;
		_g = p;
		_b = v;
		break;

	default:
		_r = v;
		_g = p;
		_b = q;
		break;
	}

	return Vec3((unsigned char)(_r * 255), (unsigned char)(_g * 255), (unsigned char)(_b * 255));
}

int RandomInteger(const int min, const int max)
{
	std::random_device rdd;
	std::mt19937 rnd(rdd());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(rnd);
}


