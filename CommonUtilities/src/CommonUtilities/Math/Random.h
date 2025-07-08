#pragma once
#include <random>
#include <cstdint>
#include "Vector/Vector.h"

#define GetRand staticDistribution(staticRandomEngine)

namespace CU
{
	class Random
	{
	public:
		Random() = delete;
		~Random() = delete;

		static void Init()
		{
			staticRandomEngine.seed(std::random_device()());
		}

		static bool Bool()
		{
			return Int() > 0 ? true : false;
		}

		static int32_t Int()
		{
			return GetRand;
		}

		static int32_t Int(int32_t aMin, int32_t aMax)
		{
			return aMin + (GetRand % (aMax - aMin + 1));
		}

		static uint32_t UInt()
		{
			return GetRand;
		}

		static uint32_t UInt(uint32_t aMin, uint32_t aMax)
		{
			return aMin + (GetRand % (aMax - aMin + 1));
		}

		static float Float01()
		{
			return (float)GetRand / (float)0xFFFFFFFF;
		}

		static float Float(float aMin, float aMax)
		{
			return Float01() * (aMax - aMin) + aMin;
		}

		static Vector3f Vector3(float aMin, float aMax)
		{
			return CU::Vector3f(Float(aMin, aMax), Float(aMin, aMax), Float(aMin, aMax));
		}

		template <typename T>
		static const T& VectorValue(const std::vector<T>& aVector)
		{
			return aVector[UInt(0, aVector.size() - 1)];
		}

	private:
		inline static thread_local std::mt19937 staticRandomEngine;
		inline static thread_local std::uniform_int_distribution<std::mt19937::result_type> staticDistribution;
	};
}
