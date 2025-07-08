#pragma once
#include <string>

namespace CU
{
	template<typename T>
	class Vector3;
	template<typename T>
	class Vector4;

	class Color
	{
	public:
		float r;
		float g;
		float b;
		float a;

		Color();
		Color(float aRGB, float aA = 1);
		Color(float aR, float aG, float aB, float aA = 1);
		Color(const std::string& aHex, float aAlpha = 1);
		Color(const Vector3<float>& aVector, float aAlpha = 1);
		Color(const Vector4<float>& aVector);
		~Color() = default;

		Color& operator=(const Color& aColor) = default;
		
		static Color Lerp(const Color& aFrom, const Color& aTo, float aLerpFactor);

		Vector3<float> GetVector3() const;
		Vector4<float> GetVector4() const;
		std::string GetHexString() const;
		unsigned GetHex() const;
		unsigned GetABGRHex() const;

		static Color GetRandom();

		static const Color Zero;
		static const Color One;
		static const Color White;
		static const Color Black;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Cyan;
		static const Color Magenta;
		static const Color Yellow;
		static const Color Orange;
	};

	inline Color operator+(const Color& aColor0, const Color& aColor1)
	{
		return { aColor0.r + aColor1.r, aColor0.g + aColor1.g, aColor0.b + aColor1.b, aColor0.a + aColor1.a };
	}

	inline Color operator-(const Color& aColor0, const Color& aColor1)
	{
		return { aColor0.r - aColor1.r, aColor0.g - aColor1.g, aColor0.b - aColor1.b, aColor0.a - aColor1.a };
	}

	inline Color operator*(const Color& aColor0, const Color& aColor1)
	{
		return { aColor0.r * aColor1.r, aColor0.g * aColor1.g, aColor0.b * aColor1.b, aColor0.a * aColor1.a };
	}

	inline Color operator*(const Color& aColor, float aScaler)
	{
		return { aColor.r * aScaler, aColor.g * aScaler, aColor.b * aScaler, aColor.a * aScaler };
	}

	inline void operator+=(Color& aColor0, const Color& aColor1)
	{
		aColor0 = aColor0 + aColor1;
	}

	inline void operator-=(Color& aColor0, const Color& aColor1)
	{
		aColor0 = aColor0 - aColor1;
	}

	inline void operator*=(Color& aColor0, const Color& aColor1)
	{
		aColor0 = aColor0 * aColor1;
	}

	inline void operator*=(Color& aColor, float aScaler)
	{
		aColor = aColor * aScaler;
	}

	inline const Color Color::Zero(0.0f, 0.0f, 0.0f, 0.0f);
	inline const Color Color::One(1.0f, 1.f, 1.f, 1.f);
	inline const Color Color::Black(0.0f, 0.0f, 0.0f, 1.f);
	inline const Color Color::White(1.f, 1.f, 1.f, 1.f);
	inline const Color Color::Red(1.f, 0.0f, 0.0f, 1.f);
	inline const Color Color::Green(0.0f, 1.f, 0.0f, 1.f);
	inline const Color Color::Blue(0.0f, 0.0f, 1.f, 1.f);
	inline const Color Color::Cyan(0.0f, 1.f, 1.f, 1.f);
	inline const Color Color::Magenta(1.f, 0.0f, 1.f, 1.f);
	inline const Color Color::Yellow(1.0f, 1.0f, 0.0f, 1.0f);
	inline const Color Color::Orange(1.0f, 0.65f, 0.0f, 1.0f);

	typedef Color Colour;
}
