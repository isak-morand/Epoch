#pragma once
#include "Vector/Vector3.hpp"
#include "CommonMath.hpp"

namespace CU
{
	template<typename T>
	class Matrix3x3;

	template<typename T>
	class Matrix4x4;

	template<typename T>
	class Quaternion
	{
	public:
		T w;
		T x;
		T y;
		T z;

		Quaternion();
		Quaternion<T>(T aW, T aX, T aY, T aZ);
		Quaternion<T>(const Vector3<T>& aVector, T aAngle);
		Quaternion<T>(T aPitch, T aYaw, T aRoll);
		Quaternion<T>(const Vector3<T>& aPitchYawRoll);
		Quaternion<T>(const Matrix4x4<T>& aRotationMatrix);
		Quaternion<T>(const Matrix3x3<T>& aRotationMatrix);
		Quaternion<T>(const Quaternion<T>& aQuaternion) = default;
		~Quaternion() = default;

		Quaternion<T>& operator=(const Quaternion<T>& aQuaternion) = default;
		Quaternion<T> operator-() const;

		Vector3<T> GetEulerAngles() const;

		Matrix3x3<T> GetRotationMatrix3x3() const;
		Matrix4x4<T> GetRotationMatrix4x4f() const;

		Vector3<T> GetRight() const;
		Vector3<T> GetUp() const;
		Vector3<T> GetForward() const;

		T LengthSqr() const;
		T Length() const;

		Quaternion<T> GetNormalized() const;
		void Normalize();
		Quaternion<T> GetConjugate() const;

		T Dot(const Quaternion<T>& aQuaternion) const;

		static Vector3<T> RotateVectorByQuaternion(const Vector3<T>& aVector, const Quaternion<T>& aQuaternion);

		static Quaternion<T> Lerp(const Quaternion<T>& aFrom, const Quaternion<T>& aTo, float aLerpFactor);
		static Quaternion<T> Slerp(const Quaternion<T>& aFrom, const Quaternion<T>& aTo, float aLerpFactor);
	};

	template <class T>
	Quaternion<T> operator*(const Quaternion<T>& aQuat0, const Quaternion<T>& aQuat1)
	{
		return Quaternion<T>(
			(aQuat0.w * aQuat1.w) - (aQuat0.x * aQuat1.x) - (aQuat0.y * aQuat1.y) - (aQuat0.z * aQuat1.z),
			(aQuat0.w * aQuat1.x) + (aQuat0.x * aQuat1.w) + (aQuat0.y * aQuat1.z) - (aQuat0.z * aQuat1.y),
			(aQuat0.w * aQuat1.y) + (aQuat0.y * aQuat1.w) + (aQuat0.z * aQuat1.x) - (aQuat0.x * aQuat1.z),
			(aQuat0.w * aQuat1.z) + (aQuat0.z * aQuat1.w) + (aQuat0.x * aQuat1.y) - (aQuat0.y * aQuat1.x)
			);
	}

	template <class T>
	void operator*=(Quaternion<T>& aQuat0, const Quaternion<T>& aQuat1)
	{
		aQuat0 = aQuat0 * aQuat1;
	}


	template<typename T>
	inline Quaternion<T>::Quaternion()
	{
		w = T(1);
		x = T(0);
		y = T(0);
		z = T(0);
	}

	template<typename T>
	inline Quaternion<T>::Quaternion(T aW, T aX, T aY, T aZ)
	{
		w = aW;
		x = aX;
		y = aY;
		z = aZ;
	}

	template<typename T>
	inline Quaternion<T>::Quaternion(const Vector3<T>& aVector, T aAngle)
	{
		const T halfAngle = aAngle / T(2);
		w = std::cos(halfAngle);

		const T halfAngleSin = std::sin(halfAngle);
		x = aVector.x * halfAngleSin;
		y = aVector.y * halfAngleSin;
		z = aVector.z * halfAngleSin;
	}

	template<typename T>
	inline Quaternion<T>::Quaternion(T aPitch, T aYaw, T aRoll) : Quaternion<T>(Vector3<T>(aPitch, aYaw, aRoll)) {}

	template<typename T>
	inline Quaternion<T>::Quaternion(const Vector3<T>& aPitchYawRoll)
	{
		const T cp = cos(aPitchYawRoll.x * static_cast<T>(0.5f));
		const T sp = sin(aPitchYawRoll.x * static_cast<T>(0.5f));
		const T ch = cos(aPitchYawRoll.y * static_cast<T>(0.5f));
		const T sh = sin(aPitchYawRoll.y * static_cast<T>(0.5f));
		const T cb = cos(aPitchYawRoll.z * static_cast<T>(0.5f));
		const T sb = sin(aPitchYawRoll.z * static_cast<T>(0.5f));

		w = ch * cp * cb + sh * sp * sb;
		x = ch * sp * cb + sh * cp * sb;
		y = sh * cp * cb - ch * sp * sb;
		z = ch * cp * sb - sh * sp * cb;
	}

	template<typename T>
	inline Quaternion<T>::Quaternion(const Matrix4x4<T>& aRotationMatrix) : Quaternion<T>(Matrix3x3<T>(aRotationMatrix))
	{
	}

	template<typename T>
	inline Quaternion<T>::Quaternion(const Matrix3x3<T>& aRotationMatrix)
	{
		const float fourWSquaredMinus1 = aRotationMatrix(1, 1) + aRotationMatrix(2, 2) + aRotationMatrix(3, 3);
		const float fourXSquaredMinus1 = aRotationMatrix(1, 1) - aRotationMatrix(2, 2) - aRotationMatrix(3, 3);
		const float fourYSquaredMinus1 = aRotationMatrix(2, 2) - aRotationMatrix(1, 1) - aRotationMatrix(3, 3);
		const float fourZSquaredMinus1 = aRotationMatrix(3, 3) - aRotationMatrix(1, 1) - aRotationMatrix(2, 2);

		int biggestIndex = 0;
		float fourBiggestSquaredMinus1 = fourWSquaredMinus1;

		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}

		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}

		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		const float biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
		const float mult = 0.25f / biggestVal;

		switch (biggestIndex)
		{
		case 0:
			w = biggestVal;
			x = (aRotationMatrix(2, 3) - aRotationMatrix(3, 2)) * mult;
			y = (aRotationMatrix(3, 1) - aRotationMatrix(1, 3)) * mult;
			z = (aRotationMatrix(1, 2) - aRotationMatrix(2, 1)) * mult;
			break;
		case 1:
			x = biggestVal;
			w = (aRotationMatrix(2, 3) - aRotationMatrix(3, 2)) * mult;
			y = (aRotationMatrix(1, 2) + aRotationMatrix(2, 1)) * mult;
			z = (aRotationMatrix(3, 1) + aRotationMatrix(1, 3)) * mult;
			break;
		case 2:
			y = biggestVal;
			w = (aRotationMatrix(3, 1) - aRotationMatrix(1, 3)) * mult;
			x = (aRotationMatrix(1, 2) + aRotationMatrix(2, 1)) * mult;
			z = (aRotationMatrix(2, 3) + aRotationMatrix(3, 2)) * mult;
			break;
		case 3:
			z = biggestVal;
			w = (aRotationMatrix(1, 2) - aRotationMatrix(2, 1)) * mult;
			x = (aRotationMatrix(3, 1) + aRotationMatrix(1, 3)) * mult;
			y = (aRotationMatrix(2, 3) + aRotationMatrix(3, 2)) * mult;
			break;
		default:
			w = T(1);
			x = T(0);
			y = T(0);
			z = T(0);
			break;
		}
	}

	template<typename T>
	inline Quaternion<T> Quaternion<T>::operator-() const
	{
		return { -w, -x, -y, -z };
	}

	template<typename T>
	inline Vector3<T> Quaternion<T>::GetEulerAngles() const
	{
		T p = T(0);
		T h = T(0);
		T b = T(0);

		const T sp = T(-2.0f) * (y * z - w * x);

		if (std::fabs(sp) > 0.9999f)
		{
			p = T(Math::HalfPi) * sp;
			h = std::atan2(-x * z + w * y, T(0.5f) - y * y - z * z);
			b = 0.0f;
		}
		else
		{
			p = std::asin(sp);
			h = std::atan2(x * z + w * y, T(0.5f) - x * x - y * y);
			b = std::atan2(x * y + w * z, T(0.5f) - x * x - z * z);
		}

		return Vector3<T>(p, h, b);
	}

	template<typename T>
	inline Matrix3x3<T> Quaternion<T>::GetRotationMatrix3x3() const
	{
		return Matrix3x3<T>(GetRotationMatrix4x4f());
	}

	template<typename T>
	inline Matrix4x4<T> Quaternion<T>::GetRotationMatrix4x4f() const
	{
		Matrix4x4<T> result = Matrix4x4f::Identity;

		const Vector3<T> right = GetRight();
		result[0] = right.x;
		result[1] = right.y;
		result[2] = right.z;

		const Vector3<T> up = GetUp();
		result[4] = up.x;
		result[5] = up.y;
		result[6] = up.z;

		const Vector3<T> forward = GetForward();
		result[8] = forward.x;
		result[9] = forward.y;
		result[10] = forward.z;

		return result;
	}

	template<typename T>
	inline Vector3<T> Quaternion<T>::GetRight() const
	{
		// https://www.gamedev.net/forums/topic/56471-extracting-direction-vectors-from-quaternion/

		return Vector3<T>(
			T(1) - T(2) * (y * y + z * z),
			T(2) * (x * y + w * z),
			T(2) * (x * z - w * y)
			).GetNormalized();
	}

	template<typename T>
	inline Vector3<T> Quaternion<T>::GetUp() const
	{
		// https://www.gamedev.net/forums/topic/56471-extracting-direction-vectors-from-quaternion/

		return Vector3<T>(
			T(2) * (x * y - w * z),
			T(1) - T(2) * (x * x + z * z),
			T(2) * (y * z + w * x)
			).GetNormalized();
	}

	template<typename T>
	inline Vector3<T> Quaternion<T>::GetForward() const
	{
		// https://www.gamedev.net/forums/topic/56471-extracting-direction-vectors-from-quaternion/

		return Vector3<T>(
			T(2) * (x * z + w * y),
			T(2) * (y * z - w * x),
			T(1) - T(2) * (x * x + y * y)
			).GetNormalized();
	}

	template<typename T>
	inline T Quaternion<T>::LengthSqr() const
	{
		return (w * w) + (x * x) + (y * y) + (z * z);
	}

	template<typename T>
	inline T Quaternion<T>::Length() const
	{
		return static_cast<T>(sqrt((w * w) + (x * x) + (y * y) + (z * z)));
	}

	template<typename T>
	inline Quaternion<T> Quaternion<T>::GetNormalized() const
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return { 0, 0, 0 };
		}

		const T magnitude = static_cast<T>(1) / Length();
		return Quaternion<T>(w * magnitude, x * magnitude, y * magnitude, z * magnitude);
	}

	template<typename T>
	inline void Quaternion<T>::Normalize()
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return;
		}

		const T magnitude = static_cast<T>(1) / Length();
		w *= magnitude;
		x *= magnitude;
		y *= magnitude;
		z *= magnitude;
	}

	template<typename T>
	inline Quaternion<T> Quaternion<T>::GetConjugate() const
	{
		return Quaternion<T>(w, -x, -y, -z);
	}

	template<typename T>
	inline T Quaternion<T>::Dot(const Quaternion<T>& aQuaternion) const
	{
		return (x * aQuaternion.x) + (y * aQuaternion.y) + (z * aQuaternion.z) + (w * aQuaternion.w);
	}

	template<typename T>
	inline Vector3<T> Quaternion<T>::RotateVectorByQuaternion(const Vector3<T>& aVector, const Quaternion<T>& aQuaternion)
	{
		// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion

		const Vector3<T> u(aQuaternion.x, aQuaternion.y, aQuaternion.z);
		const T s = aQuaternion.w;

		return u * 2.0f * aVector.Dot(u) + aVector * (s * s - u.Dot(u)) + u.Cross(aVector) * 2.0f * s;
	}

	template<typename T>
	inline Quaternion<T> Quaternion<T>::Lerp(const Quaternion<T>& aFrom, const Quaternion<T>& aTo, float aLerpFactor)
	{
		aLerpFactor = Math::Clamp01(aLerpFactor);

		const float deltaInv = 1.0f - aLerpFactor;
		return Quaternion<T>(
			deltaInv * aFrom.w + aLerpFactor * aTo.w,
			deltaInv * aFrom.x + aLerpFactor * aTo.x,
			deltaInv * aFrom.y + aLerpFactor * aTo.y,
			deltaInv * aFrom.z + aLerpFactor * aTo.z
			).GetNormalized();
	}

	template<typename T>
	inline Quaternion<T> Quaternion<T>::Slerp(const Quaternion<T>& aFrom, const Quaternion<T>& aTo, float aLerpFactor)
	{
		float cosOmega = (float)aFrom.Dot(aTo);

		Quaternion<T> newTo = aTo;
		if (cosOmega < 0.0f)
		{
			newTo = -newTo;
			cosOmega = -cosOmega;
		}

		float k0, k1;
		if (cosOmega > 0.9999f)
		{

			k0 = 1.0f - aLerpFactor;
			k1 = aLerpFactor;
		}
		else
		{
			float sinOmega = std::sqrtf(1.0f - cosOmega * cosOmega);
			float omega = std::atan2(sinOmega, cosOmega);
			float oneOverSinOmega = 1.0f / sinOmega;

			k0 = std::sin((1.0f - aLerpFactor) * omega) * oneOverSinOmega;
			k1 = std::sin(aLerpFactor * omega) * oneOverSinOmega;
		}

		return Quaternion<T>(
			aFrom.w * k0 + newTo.w * k1,
			aFrom.x * k0 + newTo.x * k1,
			aFrom.y * k0 + newTo.y * k1,
			aFrom.z * k0 + newTo.z * k1
			);
	}


	typedef Quaternion<float> Quatf;
}
