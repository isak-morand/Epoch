#pragma once
#include "CommonUtilities/Math/CommonMath.hpp"

namespace CU
{
	template<typename T>
	class Vector2;
	template<typename T>
	class Vector4;

	template<typename T>
	class Vector3
	{
	public:
		T x;
		T y;
		T z;

		Vector3<T>();
		Vector3<T>(T aValue);
		Vector3<T>(T aX, T aY, T aZ);
		Vector3<T>(const Vector2<T>& aVector, T aZ = 0);
		Vector3<T>(const Vector4<T>& aVector);
		Vector3<T>(const Vector3<T>& aVector) = default;
		~Vector3<T>() = default;

		Vector3<T>& operator=(const Vector3<T>& aVector) = default;
		Vector3<T> operator-() const;

		template <class U> operator U() const;

		T LengthSqr() const;
		T Length() const;

		Vector3<T> GetNormalized() const;
		void Normalize();

		T Dot(const Vector3<T>& aVector) const;
		Vector3<T> Cross(const Vector3<T>& aVector) const;

		T Distance(const Vector3<T>& aVector);
		T DistanceSqr(const Vector3<T>& aVector);

		Vector3<T> Lerp(const Vector3<T>& aTo, float aLerpFactor);
		Vector3<T> Slerp(const Vector3<T>& aTo, float aLerpFactor);

		Vector3<T> Reflect(const Vector3<T>& aNormal);


		static T Distance(const Vector3<T>& aVector0, const Vector3<T>& aVector1);
		static T DistanceSqr(const Vector3<T>& aVector0, const Vector3<T>& aVector1);

		static Vector3<T> Lerp(const Vector3<T>& aFrom, const Vector3<T>& aTo, float aLerpFactor);
		static Vector3<T> Slerp(const Vector3<T>& aFrom, const Vector3<T>& aTo, float aLerpFactor);

		static Vector3<T> Reflect(const Vector3<T>& aVector, const Vector3<T>& aNormal);

		static T FindAngleBetweenRadians(const Vector3<T>& aVector0, const Vector3<T>& aVector1);
		static T FindAngleBetweenDegrees(const Vector3<T>& aVector0, const Vector3<T>& aVector1);


		static const Vector3<T> Zero;
		static const Vector3<T> One;
		static const Vector3<T> Right;
		static const Vector3<T> Up;
		static const Vector3<T> Forward;
	};

	template <class T> 
	Vector3<T> operator+(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return { aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z };
	}

	template <class T> 
	Vector3<T> operator-(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return { aVector0.x - aVector1.x, aVector0.y - aVector1.y, aVector0.z - aVector1.z };
	}

	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return { aVector0.x * aVector1.x, aVector0.y * aVector1.y, aVector0.z * aVector1.z };
	}

	template <class T> 
	Vector3<T> operator*(const Vector3<T>& aVector, T aScalar)
	{
		return { aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar };
	}

	template <class T>
	Vector3<T> operator*(T aScalar, const Vector3<T>& aVector)
	{
		return { aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar };
	}

	template <class T> 
	Vector3<T> operator/(const Vector3<T>& aVector, T aScalar)
	{
		if (aScalar == 0)
		{
			return aVector;
		}

		const T multiplier = 1 / aScalar;
		return { aVector.x * multiplier, aVector.y * multiplier, aVector.z * multiplier };
	}

	template <class T> 
	void operator+=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	template <class T> 
	void operator-=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	template <class T> 
	void operator*=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0 = aVector0 * aVector1;
	}

	template <class T> 
	void operator*=(Vector3<T>& aVector, T aScalar)
	{
		aVector = aVector * aScalar;
	}

	template <class T> 
	void operator/=(Vector3<T>& aVector, T aScalar)
	{
		aVector = aVector / aScalar;
	}

	template <class T>
	bool operator==(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return (CU::Math::Abs(aVector0.x - aVector1.x) < Math::Epsilon &&
				CU::Math::Abs(aVector0.y - aVector1.y) < Math::Epsilon &&
				CU::Math::Abs(aVector0.z - aVector1.z) < Math::Epsilon);
	}

	template <class T>
	bool operator!=(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return !(aVector0 == aVector1);
	}


	template<typename T>
	Vector3<T>::Vector3()
	{
		x = T(0);
		y = T(0);
		z = T(0);
	}

	template<typename T>
	inline Vector3<T>::Vector3(T aValue)
	{
		x = aValue;
		y = aValue;
		z = aValue;
	}

	template<typename T>
	inline Vector3<T>::Vector3(T aX, T aY, T aZ)
	{
		x = aX;
		y = aY;
		z = aZ;
	}

	template<typename T>
	inline Vector3<T>::Vector3(const Vector2<T>& aVector, T aZ)
	{
		x = aVector.x;
		y = aVector.y;
		z = aZ;
	}

	template<typename T>
	Vector3<T>::Vector3(const Vector4<T>& aVector)
	{
		x = aVector.x;
		y = aVector.y;
		z = aVector.z;
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::operator-() const
	{
		return { -x, -y, -z };
	}

	template<typename T> template<typename U>
	inline Vector3<T>::operator U() const
	{
		return U(x, y, z);
	}

	template<typename T>
	inline T Vector3<T>::LengthSqr() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	template<typename T>
	inline T Vector3<T>::Length() const
	{
		return static_cast<T>(std::sqrt((x * x) + (y * y) + (z * z)));
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::GetNormalized() const
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return { 0, 0, 0 };
		}

		const T magnitude = T(1) / Length();
		return { x * magnitude, y * magnitude, z * magnitude };
	}

	template<typename T>
	inline void Vector3<T>::Normalize()
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return;
		}

		const T magnitude = T(1) / Length();
		x *= magnitude;
		y *= magnitude;
		z *= magnitude;
	}

	template<typename T>
	inline T Vector3<T>::Dot(const Vector3<T>& aVector) const
	{
		return { (x * aVector.x) + (y * aVector.y) + (z * aVector.z) };
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Cross(const Vector3<T>& aVector) const
	{
		return
		{
			(y * aVector.z) - (z * aVector.y),
			(z * aVector.x) - (x * aVector.z),
			(x * aVector.y) - (y * aVector.x)
		};
	}

	template<typename T>
	inline T Vector3<T>::Distance(const Vector3<T>& aVector)
	{
		return Distance(*this, aVector);
	}

	template<typename T>
	inline T Vector3<T>::DistanceSqr(const Vector3<T>& aVector)
	{
		return DistanceSqr(*this, aVector);
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Lerp(const Vector3<T>& aTo, float aLerpFactor)
	{
		return Lerp(*this, aTo, aLerpFactor);
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Slerp(const Vector3<T>& aTo, float aLerpFactor)
	{
		return Slerp(*this, aTo, aLerpFactor);
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Reflect(const Vector3<T>& aNormal)
	{
		return Reflect(*this, aNormal);
	}


	template<typename T>
	inline T Vector3<T>::Distance(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return T((aVector1 - aVector0).Length());
	}

	template<typename T>
	inline T Vector3<T>::DistanceSqr(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return T((aVector1 - aVector0).LengthSqr());
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Lerp(const Vector3<T>& aFrom, const Vector3<T>& aTo, float aLerpFactor)
	{
		return aFrom + (aTo - aFrom) * aLerpFactor;
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Slerp(const Vector3<T>& aFrom, const Vector3<T>& aTo, float aLerpFactor)
	{
		// https://stackoverflow.com/questions/67919193/how-does-unity-implements-vector3-slerp-exactly

		float dot = aFrom.Dot(aTo);
		dot = Math::Clamp(dot, -1.0f, 1.0f);

		float theta = acos(dot) * aLerpFactor;
		Vector3<T> relativeVec = aTo - aFrom * dot;
		relativeVec.Normalize();

		return ((aFrom * std::cos(theta)) + (relativeVec * std::sin(theta)));
	}

	template<typename T>
	inline Vector3<T> Vector3<T>::Reflect(const Vector3<T>& aVector, const Vector3<T>& aNormal)
	{
		return Vector3<T>(aVector - aNormal * aVector.Dot(aNormal) * 2.f).GetNormalized();
	}

	template<typename T>
	inline T Vector3<T>::FindAngleBetweenRadians(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return std::acos((aVector0.Dot(aVector1)) / (aVector0.Length() * aVector1.Length()));
	}

	template<typename T>
	inline T Vector3<T>::FindAngleBetweenDegrees(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return T(Math::RadToDeg(FindAngleBetweenRadians(aVector0, aVector1)));
	}


	template<typename T>
	const Vector3<T> Vector3<T>::Zero((T)0, (T)0, (T)0);
	template<typename T>
	const Vector3<T> Vector3<T>::One((T)1, (T)1, (T)1);
	template<typename T>
	const Vector3<T> Vector3<T>::Right((T)1, (T)0, (T)0);
	template<typename T>
	const Vector3<T> Vector3<T>::Up((T)0, (T)1, (T)0);
	template<typename T>
	const Vector3<T> Vector3<T>::Forward((T)0, (T)0, (T)1);

	typedef Vector3<float> Vector3f;
	typedef Vector3<unsigned> Vector3ui;
}
