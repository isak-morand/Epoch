#pragma once
#include "CommonUtilities/Math/CommonMath.hpp"

namespace CU
{
	template<typename T>
	class Vector3;

	template<typename T>
	class Vector4
	{
	public:
		T x;
		T y;
		T z;
		T w;

		Vector4<T>();
		Vector4<T>(T aX, T aY, T aZ, T aW);
		Vector4<T>(const Vector3<T>& aVector, T aW = 0);
		Vector4<T>(const Vector4<T>& aVector) = default;
		~Vector4<T>() = default;

		Vector4<T>& operator=(const Vector4<T>& aVector) = default;
		Vector4<T> operator-() const;

		template <class U> operator U() const;

		T LengthSqr() const;
		T Length() const;

		Vector4<T> GetNormalized() const;
		void Normalize();

		T Dot(const Vector4<T>& aVector) const;

		T Distance(const Vector4<T>& aVector);
		T DistanceSqr(const Vector4<T>& aVector);

		Vector4<T> Lerp(const Vector4<T>& aTo, float aLerpFactor);


		static T Distance(const Vector4<T>& aVector0, const Vector4<T>& aVector1);
		static T DistanceSqr(const Vector4<T>& aVector0, const Vector4<T>& aVector1);

		static Vector4<T> Lerp(const Vector4<T>& aFrom, const Vector4<T>& aTo, float aLerpFactor);


		static const Vector4<T> Zero;
		static const Vector4<T> One;
	};

	template <class T> 
	Vector4<T> operator+(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return { aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z, aVector0.w + aVector1.w };
	}

	template <class T> 
	Vector4<T> operator-(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return { aVector0.x - aVector1.x, aVector0.y - aVector1.y, aVector0.z - aVector1.z, aVector0.w - aVector1.w };
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return { aVector0.x * aVector1.x, aVector0.y * aVector1.y, aVector0.z * aVector1.z, aVector0.w * aVector1.w };
	}

	template <class T> 
	Vector4<T> operator*(const Vector4<T>& aVector, T aScalar)
	{
		return { aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar, aVector.w * aScalar };
	}

	template <class T>
	Vector4<T> operator*(T aScalar, const Vector4<T>& aVector)
	{
		return { aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar, aVector.w * aScalar };
	}

	template <class T> 
	Vector4<T> operator/(const Vector4<T>& aVector, T aScalar)
	{
		if (aScalar == 0)
		{
			return aVector;
		}

		const T multiplier = T(1) / aScalar;
		return { aVector.x * multiplier, aVector.y * multiplier, aVector.z * multiplier, aVector.w * multiplier };
	}

	template <class T> 
	void operator+=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	template <class T> 
	void operator-=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	template <class T> 
	void operator*=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 * aVector1;
	}

	template <class T> 
	void operator*=(Vector4<T>& aVector, T aScalar)
	{
		aVector = aVector * aScalar;
	}

	template <class T> 
	void operator/=(Vector4<T>& aVector, T aScalar)
	{
		if (aScalar == 0)
		{
			return;
		}

		aVector = aVector / aScalar;
	}

	template <class T>
	bool operator==(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return (CU::Math::Abs(aVector0.x - aVector1.x) < Math::Epsilon &&
				CU::Math::Abs(aVector0.y - aVector1.y) < Math::Epsilon &&
				CU::Math::Abs(aVector0.z - aVector1.z) < Math::Epsilon &&
				CU::Math::Abs(aVector0.w - aVector1.w) < Math::Epsilon);
	}

	template <class T>
	bool operator!=(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return !(aVector0 == aVector1);
	}


	template<typename T>
	Vector4<T>::Vector4()
	{
		x = T(0);
		y = T(0);
		z = T(0);
		w = T(0);
	}

	template<typename T>
	inline Vector4<T>::Vector4(T aX, T aY, T aZ, T aW)
	{
		x = aX;
		y = aY;
		z = aZ;
		w = aW;
	}

	template<typename T>
	inline Vector4<T>::Vector4(const Vector3<T>& aVector, T aW)
	{
		x = aVector.x;
		y = aVector.y;
		z = aVector.z;
		w = aW;
	}

	template<typename T>
	inline Vector4<T> Vector4<T>::operator-() const
	{
		return { -x, -y, -z, -w };
	}

	template<typename T> template<typename U>
	inline Vector4<T>::operator U() const
	{
		return U(x, y, z, w);
	}

	template<typename T>
	inline T Vector4<T>::LengthSqr() const
	{
		return (x * x) + (y * y) + (z * z) + (w * w);
	}

	template<typename T>
	inline T Vector4<T>::Length() const
	{
		return static_cast<T>(std::sqrt((x * x) + (y * y) + (z * z) + (w * w)));
	}

	template<typename T>
	inline Vector4<T> Vector4<T>::GetNormalized() const
	{
		if (x == 0 && y == 0 && z == 0 && w == 0)
		{
			return { 0, 0, 0, 0 };
		}

		const T magnitude = T(1) / Length();
		return { x * magnitude, y * magnitude, z * magnitude, w * magnitude };
	}

	template<typename T>
	inline void Vector4<T>::Normalize()
	{
		if (x == 0 && y == 0 && z == 0 && w == 0)
		{
			return;
		}

		const T magnitude = T(1) / Length();
		x *= magnitude;
		y *= magnitude;
		z *= magnitude;
		w *= magnitude;
	}

	template<typename T>
	inline T Vector4<T>::Dot(const Vector4<T>& aVector) const
	{
		return (x * aVector.x) + (y * aVector.y) + (z * aVector.z) + (w * aVector.w);
	}

	template<typename T>
	inline T Vector4<T>::Distance(const Vector4<T>& aVector)
	{
		return Distance(*this, aVector);
	}

	template<typename T>
	inline T Vector4<T>::DistanceSqr(const Vector4<T>& aVector)
	{
		return DistanceSqr(*this, aVector);
	}

	template<typename T>
	inline Vector4<T> Vector4<T>::Lerp(const Vector4<T>& aTo, float aLerpFactor)
	{
		return Lerp(*this, aTo, aLerpFactor);
	}

	template<typename T>
	inline T Vector4<T>::Distance(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return T((aVector1 - aVector0).Length());
	}

	template<typename T>
	inline T Vector4<T>::DistanceSqr(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return T((aVector1 - aVector0).LengthSqr());
	}

	template<typename T>
	inline Vector4<T> Vector4<T>::Lerp(const Vector4<T>& aFrom, const Vector4<T>& aTo, float aLerpFactor)
	{
		return aFrom + (aTo - aFrom) * aLerpFactor;
	}


	template<typename T>
	const Vector4<T> Vector4<T>::Zero((T)0, (T)0, (T)0, (T)0);
	template<typename T>
	const Vector4<T> Vector4<T>::One((T)1, (T)1, (T)1, (T)1);

	typedef Vector4<float> Vector4f;
	typedef Vector4<unsigned> Vector4ui;
}
