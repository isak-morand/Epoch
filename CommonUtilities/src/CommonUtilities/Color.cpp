#include "Color.h"
#include <algorithm>
#include "Math/Random.h"
#include "Math/Vector/Vector3.hpp"
#include "Math/Vector/Vector4.hpp"

namespace CU
{
	Color::Color()
	{
		r = 0.f;
		g = 0.f;
		b = 0.f;
		a = 1.f;
	}

	Color::Color(float aRGB, float aA)
	{
		r = aRGB;
		g = aRGB;
		b = aRGB;
		a = aA;
	}

	Color::Color(float aR, float aG, float aB, float aA)
	{
		r = aR;
		g = aG;
		b = aB;
		a = aA;
	}

	Color::Color(const std::string& aHex, float aAlpha)
	{
		if (static_cast<int>(aHex.size()) != 6)
		{
			r = 0.f;
			g = 0.f;
			b = 0.f;
			a = 1.f;
			return;
		}

		const unsigned long value = std::stoul(aHex, nullptr, 16);
		static const float multiplier = 1.f / 255.f;

		r = static_cast<float>((value >> 16) & 0xff) * multiplier;
		g = static_cast<float>((value >> 8) & 0xff) * multiplier;
		b = static_cast<float>((value >> 0) & 0xff) * multiplier;
		a = aAlpha;
	}

	Color::Color(const Vector3<float>& aVector, const float aAlpha)
	{
		r = aVector.x;
		g = aVector.y;
		b = aVector.z;
		a = aAlpha;
	}

	Color::Color(const Vector4<float>& aVector)
	{
		r = aVector.x;
		g = aVector.y;
		b = aVector.z;
		a = aVector.w;
	}

	Color Color::Lerp(const Color& aFrom, const Color& aTo, float aLerpFactor)
	{
		return aFrom + (aTo - aFrom) * aLerpFactor;
	}

	Vector3f Color::GetVector3() const
	{
		return { r, g, b };
	}

	Vector4f Color::GetVector4() const
	{
		return { r, g, b, a };
	}

	std::string Color::GetHexString() const
	{
		char hex[8];
		std::snprintf(hex, sizeof(hex), "#%02x%02x%02x", static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f));
		return std::string(hex);
	}

	unsigned Color::GetHex() const
	{
		const uint8_t rVal = (uint8_t)(r * 255.0f);
		const uint8_t gVal = (uint8_t)(g * 255.0f);
		const uint8_t bVal = (uint8_t)(b * 255.0f);
		const uint8_t aVal = (uint8_t)(a * 255.0f);
		return (0x00000000 | (rVal << 24) | (gVal << 16) | (bVal << 8) | aVal);
	}

	unsigned Color::GetABGRHex() const
	{
		const uint8_t rVal = (uint8_t)(r * 255.0f);
		const uint8_t gVal = (uint8_t)(g * 255.0f);
		const uint8_t bVal = (uint8_t)(b * 255.0f);
		const uint8_t aVal = (uint8_t)(a * 255.0f);
		return (0x00000000 | (aVal << 24) | (bVal << 16) | (gVal << 8) | rVal);
	}

	Color Color::GetRandom()
	{
		return Color(Random::Float01(), Random::Float01(), Random::Float01());
	}
}