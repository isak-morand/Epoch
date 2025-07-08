#pragma once
#include <stdint.h>
#include <cmath>

namespace CU::Math
{
	constexpr float Epsilon = 0.000001f;

	constexpr float Pi = 3.141592f;
	constexpr float HalfPi = Pi / 2.0f;
	constexpr float Tau = Pi * 2;
	constexpr float Phi = 1.618033f;
	constexpr float ToDeg = 180.0f / Pi;
	constexpr float ToRad = Pi / 180.0f;


	template<typename T>
	inline T RadToDeg(T aValue) { return aValue * static_cast<T>(ToDeg); }

	template<typename T>
	inline T DegToRad(T aValue) { return aValue * static_cast<T>(ToRad); }


	template<typename T>
	inline int FloorToInt(T aValue) { return static_cast<int>(std::floorf(aValue)); }
	
	template<typename T>
	inline unsigned FloorToUInt(T aValue) { return static_cast<unsigned>(std::floorf(aValue)); }

	template<typename T>
	inline T Floor(T aValue) { return static_cast<T>(std::floorf(aValue)); }

	template<typename T>
	inline int RoundToInt(T aValue) { return static_cast<int>(std::roundf(aValue)); }
	
	template<typename T>
	inline unsigned RoundToUInt(T aValue) { return static_cast<unsigned>(std::roundf(aValue)); }

	template<typename T>
	inline T Round(T aValue) { return static_cast<T>(std::roundf(aValue)); }

	template<typename T>
	inline int CeilToInt(T aValue) { return static_cast<int>(std::ceilf(aValue)); }

	template<typename T>
	inline unsigned CeilToUInt(T aValue) { return static_cast<unsigned>(std::ceilf(aValue)); }

	template<typename T>
	inline T Ceil(T aValue) { return static_cast<T>(std::ceilf(aValue)); }


	template<typename T>
	inline T Mod(T aValue) { return aValue - static_cast<int>(aValue); }

	template<typename T>
	inline T Mod(T aValue, int& aIntPart) { aIntPart = static_cast<int>(aValue); return aValue - aIntPart; }

	template<typename T>
	inline T Min(T aValue0, T aValue1) { return (aValue0 < aValue1) ? aValue0 : aValue1; }

	template<typename T>
	inline T Max(T aValue0, T aValue1) { return (aValue0 < aValue1) ? aValue1 : aValue0; }

	template<typename T>
	inline T Abs(T aValue) { return (aValue < static_cast<T>(0)) ? static_cast<T>(aValue * static_cast<T>(-1)) : aValue; }

	template<typename T>
	inline T Sign(T aValue) { return (aValue >= static_cast<T>(0)) ? static_cast<T>(1) : static_cast<T>(-1); }


	template<typename T>
	inline T Clamp(T aValue, T aMin, T aMax) { return ((aValue > aMax) ? aMax : (aValue < aMin) ? aMin : aValue); }

	template<typename T>
	inline T Clamp01(T aValue) { return ((aValue > static_cast<T>(1)) ? static_cast<T>(1) : (aValue < static_cast<T>(0)) ? static_cast<T>(0) : aValue); }


	template<typename T>
	inline T Lerp(T aFrom, T aTo, float aLerpFactor) { return aFrom + (aTo - aFrom) * aLerpFactor; }
	
	template<typename T>
	inline float InverseLerp(T aFrom, T aTo, T aValue) { return (float)((aValue - aFrom) / (aTo - aFrom)); }

	template<typename T>
	inline T Wrap(T aValue, T aMin, T aMax) { const T delta = (aMax - aMin); while (aValue > aMax) aValue -= delta; while (aValue < aMin) aValue += delta; return aValue; }

	template<typename T>
	inline T Remap(T aValue, T aOldMin, T aOldMax, T aNewMin, T aNewMax) { return (((aValue - aOldMin) * (aNewMax - aNewMin)) / (aOldMax - aOldMin)) + aNewMin; }

	template<typename T>
	inline T Remap01(T aValue, T aOldMin, T aOldMax) { return (aValue - aOldMin) / (aOldMax - aOldMin); }
}
