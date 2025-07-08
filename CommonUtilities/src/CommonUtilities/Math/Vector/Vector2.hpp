#pragma once
#include "CommonUtilities/Math/CommonMath.hpp"

namespace CU
{
	template<typename T>
	class Vector2
	{
	public:
		T x;
		T y;

		Vector2<T>();
		Vector2<T>(T aValue);
		Vector2<T>(T aX, T aY);
		Vector2<T>(const Vector2<T>& aVector) = default;
		~Vector2<T>() = default;

		Vector2<T>& operator=(const Vector2<T>& aVector) = default;
		Vector2<T> operator-() const;

		template <class U> operator U() const;

		T LengthSqr() const;
		T Length() const;

		Vector2<T> GetNormalized() const;
		void Normalize();

		T Dot(const Vector2<T>& aVector) const;
		float Cross(const Vector2<T>& aVector) const;

		T Distance(const Vector2<T>& aVector);
		T DistanceSqr(const Vector2<T>& aVector);

		Vector2<T> Lerp(const Vector2<T>& aTo, float aLerpFactor);
		Vector2<T> Slerp(const Vector2<T>& aTo, float aLerpFactor);

		Vector2<T> Reflect(const Vector2<T>& aNormal);


		static T Distance(const Vector2<T>& aVector0, const Vector2<T>& aVector1);
		static T DistanceSqr(const Vector2<T>& aVector0, const Vector2<T>& aVector1);

		static Vector2<T> Lerp(const Vector2<T>& aFrom, const Vector2<T>& aTo, float aLerpFactor);
		static Vector2<T> Slerp(const Vector2<T>& aFrom, const Vector2<T>& aTo, float aLerpFactor);

		static Vector2<T> Reflect(const Vector2<T>& aVector, const Vector2<T>& aNormal);

		static T FindAngleBetweenRadians(const Vector2<T>& aVector0, const Vector2<T>& aVector1);
		static T FindAngleBetweenDegrees(const Vector2<T>& aVector0, const Vector2<T>& aVector1);


		static const Vector2<T> Zero;
		static const Vector2<T> One;
		static const Vector2<T> Right;
		static const Vector2<T> Up;
	};

	template <class T> 
	Vector2<T> operator+(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return { aVector0.x + aVector1.x, aVector0.y + aVector1.y };
	}

	template <class T> 
	Vector2<T> operator-(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return { aVector0.x - aVector1.x, aVector0.y - aVector1.y };
	}

	template <class T>
	Vector2<T> operator*(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return { aVector0.x * aVector1.x, aVector0.y * aVector1.y };
	}
	
	template <class T> 
	Vector2<T> operator*(const Vector2<T>& aVector, T aScalar)
	{
		return { aVector.x * aScalar, aVector.y * aScalar };
	}

	template <class T>
	Vector2<T> operator*(T aScalar, const Vector2<T>& aVector)
	{
		return { aVector.x * aScalar, aVector.y * aScalar };
	}

	template <class T> 
	Vector2<T> operator/(const Vector2<T>& aVector, T aScalar)
	{
		if (aScalar == 0)
		{
			return aVector;
		}

		const T multiplier = 1 / aScalar;
		return { aVector.x * multiplier, aVector.y * multiplier };
	}

	template <class T> 
	void operator+=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	template <class T> 
	void operator-=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	template <class T> 
	void operator*=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 * aVector1;
	}

	template <class T> 
	void operator*=(Vector2<T>& aVector, T aScalar)
	{
		aVector = aVector * aScalar;
	}

	template <class T> 
	void operator/=(Vector2<T>& aVector, T aScalar)
	{
		if (aScalar == 0)
		{
			return;
		}

		aVector = aVector / aScalar;
	}

	template <class T>
	bool operator==(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return (CU::Math::Abs(aVector0.x - aVector1.x) < Math::Epsilon &&
				CU::Math::Abs(aVector0.y - aVector1.y) < Math::Epsilon);
	}

	template <class T>
	bool operator!=(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return !(aVector0 == aVector1);
	}


	template<typename T>
	Vector2<T>::Vector2()
	{
		x = T(0);
		y = T(0);
	}

	template<typename T>
	inline Vector2<T>::Vector2(T aValue)
	{
		x = aValue;
		y = aValue;
	}

	template<typename T>
	inline Vector2<T>::Vector2(T aX, T aY)
	{
		x = aX;
		y = aY;
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::operator-() const
	{
		return { -x, -y };
	}

	template<typename T> template<typename U>
	inline Vector2<T>::operator U() const
	{
		return U(x, y);
	}

	template<typename T>
	inline T Vector2<T>::LengthSqr() const
	{
		return (x * x) + (y * y);
	}

	template<typename T>
	inline T Vector2<T>::Length() const
	{
		return static_cast<T>(std::sqrt((x * x) + (y * y)));
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::GetNormalized() const
	{
		if (x == 0 && y == 0)
		{
			return { 0, 0 };
		}

		const T magnitude = T(1) / Length();
		return { x * magnitude, y * magnitude };
	}

	template<typename T>
	inline void Vector2<T>::Normalize()
	{
		if (x == 0 && y == 0)
		{
			return;
		}

		const T magnitude = T(1) / Length();
		x *= magnitude;
		y *= magnitude;
	}

	template<typename T>
	inline T Vector2<T>::Dot(const Vector2<T>& aVector) const
	{
		return (x * aVector.x) + (y * aVector.y);
	}

	template<typename T>
	inline float Vector2<T>::Cross(const Vector2<T>& aVector) const
	{
		return x * aVector.y - y * aVector.x;
	}

	template<typename T>
	inline T Vector2<T>::Distance(const Vector2<T>& aVector)
	{
		return Distance(*this, aVector);
	}

	template<typename T>
	inline T Vector2<T>::DistanceSqr(const Vector2<T>& aVector)
	{
		return DistanceSqr(*this, aVector);
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Lerp(const Vector2<T>& aTo, float aLerpFactor)
	{
		return Lerp(*this, aTo, aLerpFactor);
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Slerp(const Vector2<T>& aTo, float aLerpFactor)
	{
		return Slerp(*this, aTo, aLerpFactor);
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Reflect(const Vector2<T>& aNormal)
	{
		return Reflect(*this, aNormal);
	}


	template<typename T>
	inline T Vector2<T>::Distance(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return T((aVector1 - aVector0).Length());
	}

	template<typename T>
	inline T Vector2<T>::DistanceSqr(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return T((aVector1 - aVector0).LengthSqr());
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Lerp(const Vector2<T>& aFrom, const Vector2<T>& aTo, float aLerpFactor)
	{
		return aFrom + (aTo - aFrom) * aLerpFactor;
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Slerp(const Vector2<T>& aFrom, const Vector2<T>& aTo, float aLerpFactor)
	{
		// https://stackoverflow.com/questions/67919193/how-does-unity-implements-vector3-slerp-exactly

		float dot = aFrom.Dot(aTo);
		dot = Math::Clamp(dot, -1.0f, 1.0f);

		float theta = acos(dot) * Math::Clamp01(aLerpFactor);
		Vector2<T> relativeVec = aTo - aFrom * dot;
		relativeVec.Normalize();

		return ((aFrom * std::cos(theta)) + (relativeVec * std::sin(theta)));
	}

	template<typename T>
	inline Vector2<T> Vector2<T>::Reflect(const Vector2<T>& aVector, const Vector2<T>& aNormal)
	{
		return Vector2<T>(aVector - aNormal * aVector.Dot(aNormal) * 2.f).GetNormalized();
	}

	template<typename T>
	inline T Vector2<T>::FindAngleBetweenRadians(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return std::acos((aVector0.Dot(aVector1)) / (aVector0.Length() * aVector1.Length()));
	}

	template<typename T>
	inline T Vector2<T>::FindAngleBetweenDegrees(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return T(Math::RadToDeg(FindAngelBetweenInRadians(aVector0, aVector1)));
	}


	template<typename T>
	const Vector2<T> Vector2<T>::Zero((T)0, (T)0);
	template<typename T>
	const Vector2<T> Vector2<T>::One((T)1, (T)1);
	template<typename T>
	const Vector2<T> Vector2<T>::Right((T)1, (T)0);
	template<typename T>
	const Vector2<T> Vector2<T>::Up((T)0, (T)1);

	typedef Vector2<float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector2<unsigned int> Vector2ui;
}
