#pragma once
#include <array>
#include <algorithm>
//#include <dvec.h>
#include "Matrix3x3.hpp"
#include "CommonUtilities/Math/CommonMath.hpp"
#include "CommonUtilities/Math/Vector/Vector3.hpp"
#include "CommonUtilities/Math/Vector/Vector4.hpp"

namespace CU
{
	template<typename T>
	class Quaternion;

	template<typename T>
	class Matrix4x4
	{
	public:
		Matrix4x4<T>();
		Matrix4x4<T>(const Matrix4x4<T>& aMatrix);
		Matrix4x4<T>(std::initializer_list<T> aList);
		Matrix4x4<T>(const T aArray[16]);
		~Matrix4x4<T>() = default;

		T& operator()(const int aRow, const int aColumn);
		const T& operator()(const int aRow, const int aColumn) const;

		T& operator[](const int aIndex);
		const T& operator[](const int aIndex) const;

		Matrix4x4<T>& operator=(const Matrix4x4<T>& aMatrix);

		Matrix4x4<T> GetTranspose() const;
		void Transpose();

		Matrix4x4<T> GetInverse() const;
		Matrix4x4<T> GetFastInverse() const;

		CU::Vector4<T> GetRow(uint32_t aRow) const;
		CU::Vector4<T> GetColumn(uint32_t aColumn) const;

		CU::Vector4<T> GetRight() const;
		CU::Vector4<T> GetUp() const;
		CU::Vector4<T> GetForward() const;

		CU::Vector4<T> GetTranslation() const;
		CU::Vector4<T> GetRotation() const;
		Quaternion<T> GetOrientation() const;
		CU::Vector3<T> GetScale() const;

		void Decompose(Vector3<T>& aPosition, Vector3<T>& aRotation, Vector3<T>& aScale) const;
		void Decompose(Vector3<T>& aPosition, Quaternion<T>& aOrientaion, Vector3<T>& aScale) const;

		static Matrix4x4<T> CreateRotationAroundX(const T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundY(const T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundZ(const T aAngleInRadians);

		static Matrix4x4<T> LookAt(const Vector3<T>& aPosition, const Vector3<T>& aTarget, const Vector3<T>& aUp = Vector3<T>::Up);

		static Matrix4x4<T> CreatePerspectiveProjection(float aFOV, float aNearPlane, float aFarPlane, float aAspectRatio);
		static Matrix4x4<T> CreateOrthographicProjection(float aLeftPlane, float aRightPlane, float aBottomPlane, float aTopPlane, float aNearPlane = (T)0, float aFarPlane = (T)1);

		static Matrix4x4<T> CreateScaleMatrix(const Vector3<T>& aScale);
		static Matrix4x4<T> CreateRotationMatrix(const Vector3<T>& aRight, const Vector3<T>& aUp, const Vector3<T>& aForward);
		static Matrix4x4<T> CreateRotationMatrix(const Vector3<T>& aRotation);
		static Matrix4x4<T> CreateTranslationMatrix(const Vector3<T>& aTranslation);

		static const Matrix4x4<T> Zero;
		static const Matrix4x4<T> Identity;

	private:
#pragma warning( disable : 4201 )
		union
		{
			std::array<T, 16> myData;

			//struct
			//{
			//	__m128 m1;
			//	__m128 m2;
			//	__m128 m3;
			//	__m128 m4;
			//};
			struct
			{
				T m11;
				T m12;
				T m13;
				T m14;

				T m21;
				T m22;
				T m23;
				T m24;

				T m31;
				T m32;
				T m33;
				T m34;

				T m41;
				T m42;
				T m43;
				T m44;
			};
		};
#pragma warning( default : 4201 )
	};

	template <class T>
	Matrix4x4<T> operator+(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		return
		{
			aMatrix0[0] + aMatrix1[0],
			aMatrix0[1] + aMatrix1[1],
			aMatrix0[2] + aMatrix1[2],
			aMatrix0[3] + aMatrix1[3],

			aMatrix0[4] + aMatrix1[4],
			aMatrix0[5] + aMatrix1[5],
			aMatrix0[6] + aMatrix1[6],
			aMatrix0[7] + aMatrix1[7],

			aMatrix0[8] + aMatrix1[8],
			aMatrix0[9] + aMatrix1[9],
			aMatrix0[10] + aMatrix1[10],
			aMatrix0[11] + aMatrix1[11],

			aMatrix0[12] + aMatrix1[12],
			aMatrix0[13] + aMatrix1[13],
			aMatrix0[14] + aMatrix1[14],
			aMatrix0[15] + aMatrix1[15],
		};
	}

	template <class T>
	Matrix4x4<T> operator-(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		return
		{
			aMatrix0[0] - aMatrix1[0],
			aMatrix0[1] - aMatrix1[1],
			aMatrix0[2] - aMatrix1[2],
			aMatrix0[3] - aMatrix1[3],

			aMatrix0[4] - aMatrix1[4],
			aMatrix0[5] - aMatrix1[5],
			aMatrix0[6] - aMatrix1[6],
			aMatrix0[7] - aMatrix1[7],

			aMatrix0[8] - aMatrix1[8],
			aMatrix0[9] - aMatrix1[9],
			aMatrix0[10] - aMatrix1[10],
			aMatrix0[11] - aMatrix1[11],

			aMatrix0[12] - aMatrix1[12],
			aMatrix0[13] - aMatrix1[13],
			aMatrix0[14] - aMatrix1[14],
			aMatrix0[15] - aMatrix1[15],
		};
	}

	template <class T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		/*Matrix4x4<float> result;
		const __m128& m1 = aMatrix1.m1;
		const __m128& m2 = aMatrix1.m2;
		const __m128& m3 = aMatrix1.m3;
		const __m128& m4 = aMatrix1.m4;

		__m128 t1, t2;

		t1 = _mm_set1_ps(aMatrix0.m11);
		t2 = _mm_mul_ps(m1, t1);
		t1 = _mm_set1_ps(aMatrix0.m12);
		t2 = _mm_add_ps(_mm_mul_ps(m2, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m13);
		t2 = _mm_add_ps(_mm_mul_ps(m3, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m14);
		t2 = _mm_add_ps(_mm_mul_ps(m4, t1), t2);

		_mm_store_ps(&result.m11, t2);

		t1 = _mm_set1_ps(aMatrix0.m21);
		t2 = _mm_mul_ps(m1, t1);
		t1 = _mm_set1_ps(aMatrix0.m22);
		t2 = _mm_add_ps(_mm_mul_ps(m2, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m23);
		t2 = _mm_add_ps(_mm_mul_ps(m3, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m24);
		t2 = _mm_add_ps(_mm_mul_ps(m4, t1), t2);

		_mm_store_ps(&result.m21, t2);

		t1 = _mm_set1_ps(aMatrix0.m31);
		t2 = _mm_mul_ps(m1, t1);
		t1 = _mm_set1_ps(aMatrix0.m32);
		t2 = _mm_add_ps(_mm_mul_ps(m2, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m33);
		t2 = _mm_add_ps(_mm_mul_ps(m3, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m34);
		t2 = _mm_add_ps(_mm_mul_ps(m4, t1), t2);

		_mm_store_ps(&result.m31, t2);

		t1 = _mm_set1_ps(aMatrix0.m41);
		t2 = _mm_mul_ps(m1, t1);
		t1 = _mm_set1_ps(aMatrix0.m42);
		t2 = _mm_add_ps(_mm_mul_ps(m2, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m43);
		t2 = _mm_add_ps(_mm_mul_ps(m3, t1), t2);
		t1 = _mm_set1_ps(aMatrix0.m44);
		t2 = _mm_add_ps(_mm_mul_ps(m4, t1), t2);

		_mm_store_ps(&result.m41, t2);
		return result;*/


		Matrix4x4<T> result;

		result[0] =
			aMatrix0[0] * aMatrix1[0] +
			aMatrix0[1] * aMatrix1[4] +
			aMatrix0[2] * aMatrix1[8] +
			aMatrix0[3] * aMatrix1[12];

		result[1] =
			aMatrix0[0] * aMatrix1[1] +
			aMatrix0[1] * aMatrix1[5] +
			aMatrix0[2] * aMatrix1[9] +
			aMatrix0[3] * aMatrix1[13];

		result[2] =
			aMatrix0[0] * aMatrix1[2] +
			aMatrix0[1] * aMatrix1[6] +
			aMatrix0[2] * aMatrix1[10] +
			aMatrix0[3] * aMatrix1[14];

		result[3] =
			aMatrix0[0] * aMatrix1[3] +
			aMatrix0[1] * aMatrix1[7] +
			aMatrix0[2] * aMatrix1[11] +
			aMatrix0[3] * aMatrix1[15];


		result[4] =
			aMatrix0[4] * aMatrix1[0] +
			aMatrix0[5] * aMatrix1[4] +
			aMatrix0[6] * aMatrix1[8] +
			aMatrix0[7] * aMatrix1[12];

		result[5] =
			aMatrix0[4] * aMatrix1[1] +
			aMatrix0[5] * aMatrix1[5] +
			aMatrix0[6] * aMatrix1[9] +
			aMatrix0[7] * aMatrix1[13];

		result[6] =
			aMatrix0[4] * aMatrix1[2] +
			aMatrix0[5] * aMatrix1[6] +
			aMatrix0[6] * aMatrix1[10] +
			aMatrix0[7] * aMatrix1[14];

		result[7] =
			aMatrix0[4] * aMatrix1[3] +
			aMatrix0[5] * aMatrix1[7] +
			aMatrix0[6] * aMatrix1[11] +
			aMatrix0[7] * aMatrix1[15];


		result[8] =
			aMatrix0[8] * aMatrix1[0] +
			aMatrix0[9] * aMatrix1[4] +
			aMatrix0[10] * aMatrix1[8] +
			aMatrix0[11] * aMatrix1[12];

		result[9] =
			aMatrix0[8] * aMatrix1[1] +
			aMatrix0[9] * aMatrix1[5] +
			aMatrix0[10] * aMatrix1[9] +
			aMatrix0[11] * aMatrix1[13];

		result[10] =
			aMatrix0[8] * aMatrix1[2] +
			aMatrix0[9] * aMatrix1[6] +
			aMatrix0[10] * aMatrix1[10] +
			aMatrix0[11] * aMatrix1[14];

		result[11] =
			aMatrix0[8] * aMatrix1[3] +
			aMatrix0[9] * aMatrix1[7] +
			aMatrix0[10] * aMatrix1[11] +
			aMatrix0[11] * aMatrix1[15];


		result[12] =
			aMatrix0[12] * aMatrix1[0] +
			aMatrix0[13] * aMatrix1[4] +
			aMatrix0[14] * aMatrix1[8] +
			aMatrix0[15] * aMatrix1[12];

		result[13] =
			aMatrix0[12] * aMatrix1[1] +
			aMatrix0[13] * aMatrix1[5] +
			aMatrix0[14] * aMatrix1[9] +
			aMatrix0[15] * aMatrix1[13];

		result[14] =
			aMatrix0[12] * aMatrix1[2] +
			aMatrix0[13] * aMatrix1[6] +
			aMatrix0[14] * aMatrix1[10] +
			aMatrix0[15] * aMatrix1[14];

		result[15] =
			aMatrix0[12] * aMatrix1[3] +
			aMatrix0[13] * aMatrix1[7] +
			aMatrix0[14] * aMatrix1[11] +
			aMatrix0[15] * aMatrix1[15];

		return result;
	}

	template <class T>
	Vector4<T> operator*(const Matrix4x4<T>& aMatrix, const Vector4<T>& aVector)
	{
		return
		{
			aVector.x * aMatrix[0] + aVector.y * aMatrix[4] + aVector.z * aMatrix[8] + aVector.w * aMatrix[12],
			aVector.x * aMatrix[1] + aVector.y * aMatrix[5] + aVector.z * aMatrix[9] + aVector.w * aMatrix[13],
			aVector.x * aMatrix[2] + aVector.y * aMatrix[6] + aVector.z * aMatrix[10] + aVector.w * aMatrix[14],
			aVector.x * aMatrix[3] + aVector.y * aMatrix[7] + aVector.z * aMatrix[11] + aVector.w * aMatrix[15]
		};
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		return
		{
			aVector.x * aMatrix[0] + aVector.y * aMatrix[4] + aVector.z * aMatrix[8] + aVector.w * aMatrix[12],
			aVector.x * aMatrix[1] + aVector.y * aMatrix[5] + aVector.z * aMatrix[9] + aVector.w * aMatrix[13],
			aVector.x * aMatrix[2] + aVector.y * aMatrix[6] + aVector.z * aMatrix[10] + aVector.w * aMatrix[14],
			aVector.x * aMatrix[3] + aVector.y * aMatrix[7] + aVector.z * aMatrix[11] + aVector.w * aMatrix[15]
		};
	}

	template <class T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix, T aScalar)
	{
		return
		{
			aMatrix[0] * aScalar,
			aMatrix[1] * aScalar,
			aMatrix[2] * aScalar,
			aMatrix[3] * aScalar,

			aMatrix[4] * aScalar,
			aMatrix[5] * aScalar,
			aMatrix[6] * aScalar,
			aMatrix[7] * aScalar,

			aMatrix[8] * aScalar,
			aMatrix[9] * aScalar,
			aMatrix[10] * aScalar,
			aMatrix[11] * aScalar,

			aMatrix[12] * aScalar,
			aMatrix[13] * aScalar,
			aMatrix[14] * aScalar,
			aMatrix[15] * aScalar
		};
	}

	template <class T>
	Matrix4x4<T> operator*(T aScalar, const Matrix4x4<T>& aMatrix)
	{
		return
		{
			aMatrix[0] * aScalar,
			aMatrix[1] * aScalar,
			aMatrix[2] * aScalar,
			aMatrix[3] * aScalar,

			aMatrix[4] * aScalar,
			aMatrix[5] * aScalar,
			aMatrix[6] * aScalar,
			aMatrix[7] * aScalar,

			aMatrix[8] * aScalar,
			aMatrix[9] * aScalar,
			aMatrix[10] * aScalar,
			aMatrix[11] * aScalar,

			aMatrix[12] * aScalar,
			aMatrix[13] * aScalar,
			aMatrix[14] * aScalar,
			aMatrix[15] * aScalar
		};
	}

	template <class T>
	void operator+=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 + aMatrix1;
	}

	template <class T>
	void operator-=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 - aMatrix1;
	}

	template <class T>
	void operator*=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 * aMatrix1;
	}

	template <class T>
	void operator*=(const Matrix4x4<T>& aMatrix, Vector4<T>& aVector)
	{
		aVector = aMatrix * aVector;
	}

	template <class T>
	void operator*=(Matrix4x4<T>& aMatrix, const float aScalar)
	{
		aMatrix = aMatrix * aScalar;
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4()
	{
		myData =
		{
			T(1), T(0), T(0), T(0),
			T(0), T(1), T(0), T(0),
			T(0), T(0), T(1), T(0),
			T(0), T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(const Matrix4x4<T>& aMatrix)
	{
		memcpy_s(&myData.front(), sizeof(T) * 16, &aMatrix.myData.front(), sizeof(T) * 16);
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(std::initializer_list<T> aList)
	{
		memcpy_s(&myData.front(), sizeof(T) * 16, aList.begin(), sizeof(T) * 16);
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(const T aArray[16])
	{
		memcpy_s(&myData.front(), sizeof(T) * 16, &aArray[0], sizeof(T) * 16);
	}

	template<typename T>
	inline T& Matrix4x4<T>::operator()(const int aRow, const int aColumn)
	{
		const int index = (aRow - 1) * 4 + (aColumn - 1);
		return myData[index];
	}

	template<typename T>
	inline const T& Matrix4x4<T>::operator()(const int aRow, const int aColumn) const
	{
		const int index = (aRow - 1) * 4 + (aColumn - 1);
		return myData[index];
	}

	template<typename T>
	inline T& Matrix4x4<T>::operator[](const int aIndex)
	{
		return myData[aIndex];
	}

	template<typename T>
	inline const T& Matrix4x4<T>::operator[](const int aIndex) const
	{
		return myData[aIndex];
	}

	template<typename T>
	inline Matrix4x4<T>& Matrix4x4<T>::operator=(const Matrix4x4<T>& aMatrix)
	{
		memcpy_s(&myData.front(), sizeof(T) * 16, &aMatrix.myData.front(), sizeof(T) * 16);
		return *this;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetTranspose() const
	{
		return
		{
			myData[0], myData[4], myData[8], myData[12],
			myData[1], myData[5], myData[9], myData[13],
			myData[2], myData[6], myData[10], myData[14],
			myData[3], myData[7], myData[11], myData[15]
		};
	}

	template<typename T>
	inline void Matrix4x4<T>::Transpose()
	{
		myData =
		{
			m11, m21, m31, m41,
			m12, m22, m32, m42,
			m13, m23, m33, m43,
			m14, m24, m34, m44
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetInverse() const
	{
		Matrix4x4<T> inv;
		T determinant;
		const std::array<T, 16>& m = myData; // For easier indexing

		// Compute cofactors
		inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15]
			+ m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

		inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15]
			- m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

		inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15]
			+ m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

		inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11]
			- m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

		inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15]
			- m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

		inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15]
			+ m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

		inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15]
			- m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11]
			+ m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

		inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15]
			+ m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

		inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15]
			- m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

		inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15]
			+ m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

		inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11]
			- m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

		inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14]
			- m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

		inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14]
			+ m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

		inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14]
			- m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

		inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10]
			+ m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

		// Compute determinant
		determinant = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (determinant == T(0.0f))
		{
			return Matrix4x4<T>::Identity; // Non-invertible matrix
		}

		determinant = T(1.0f) / determinant;

		// Multiply by determinant
		for (uint32_t i = 0; i < 16; i++)
		{
			inv.myData[i] *= determinant;
		}

		return inv;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetFastInverse() const
	{
		Matrix4x4<T> result;

		result[0] = myData[0];
		result[1] = myData[4];
		result[2] = myData[8];

		result[4] = myData[1];
		result[5] = myData[5];
		result[6] = myData[9];

		result[8] = myData[2];
		result[9] = myData[6];
		result[10] = myData[10];

		result[15] = myData[15];

		result[12] = ((-myData[12]) * result[0]) + ((-myData[13]) * result[4]) + ((-myData[14]) * result[8]);
		result[13] = ((-myData[12]) * result[1]) + ((-myData[13]) * result[5]) + ((-myData[14]) * result[9]);
		result[14] = ((-myData[12]) * result[2]) + ((-myData[13]) * result[6]) + ((-myData[14]) * result[10]);

		return result;
	}

	template<typename T>
	inline CU::Vector4<T> Matrix4x4<T>::GetRow(uint32_t aRow) const
	{
		return
		{
			myData[0 + 4 * (aRow - 1)],
			myData[1 + 4 * (aRow - 1)],
			myData[2 + 4 * (aRow - 1)],
			myData[3 + 4 * (aRow - 1)]
		};
	}

	template<typename T>
	inline CU::Vector4<T> Matrix4x4<T>::GetColumn(uint32_t aColumn) const
	{
		return
		{
			myData[0 + (aColumn - 1)],
			myData[4 + (aColumn - 1)],
			myData[8 + (aColumn - 1)],
			myData[12 + (aColumn - 1)]
		};
	}

	template<typename T>
	inline CU::Vector4<T> Matrix4x4<T>::GetRight() const
	{
		return CU::Vector4<T>(m11, m12, m13, T(0)).GetNormalized();
	}

	template<typename T>
	inline CU::Vector4<T> Matrix4x4<T>::GetUp() const
	{
		return CU::Vector4<T>(m21, m22, m23, T(0)).GetNormalized();
	}

	template<typename T>
	inline CU::Vector4<T> Matrix4x4<T>::GetForward() const
	{
		return CU::Vector4<T>(m31, m32, m33, T(0)).GetNormalized();
	}

	template<typename T>
	CU::Vector4<T> Matrix4x4<T>::GetTranslation() const
	{
		return { m41, m42, m43, 1.0f };
	}

	template<typename T>
	CU::Vector4<T> Matrix4x4<T>::GetRotation() const
	{
		const Vector3<T> rightDir = { m11, m12, m13 };
		const Vector3<T> upDir = { m21, m22, m23 };
		const Vector3<T> forwardDir = { m31, m32, m33 };

		const Matrix3x3<T> rotationMatrix = Matrix4x4<T>::CreateRotationMatrix(rightDir.GetNormalized(), upDir.GetNormalized(), forwardDir.GetNormalized());
		return Quaternion<T>(rotationMatrix).GetEulerAngles();
	}

	template<typename T>
	CU::Quaternion<T> Matrix4x4<T>::GetOrientation() const
	{
		const Vector3<T> rightDir = { m11, m12, m13 };
		const Vector3<T> upDir = { m21, m22, m23 };
		const Vector3<T> forwardDir = { m31, m32, m33 };

		const Matrix3x3<T> rotationMatrix = Matrix4x4<T>::CreateRotationMatrix(rightDir.GetNormalized(), upDir.GetNormalized(), forwardDir.GetNormalized());
		return Quaternion<T>(rotationMatrix);
	}

	template<typename T>
	CU::Vector3<T> Matrix4x4<T>::GetScale() const
	{
		const Vector3<T> rightDir = { m11, m12, m13 };
		const Vector3<T> upDir = { m21, m22, m23 };
		const Vector3<T> forwardDir = { m31, m32, m33 };

		return { rightDir.Length(), upDir.Length(), forwardDir.Length() };
	}

	template<typename T>
	inline void Matrix4x4<T>::Decompose(Vector3<T>& aPosition, Vector3<T>& aRotation, Vector3<T>& aScale) const
	{
		aPosition = { m41, m42, m43 };

		const Vector3<T> rightDir = { m11, m12, m13 };
		const Vector3<T> upDir = { m21, m22, m23 };
		const Vector3<T> forwardDir = { m31, m32, m33 };

		const Matrix3x3<T> rotationMatrix = Matrix4x4<T>::CreateRotationMatrix(rightDir.GetNormalized(), upDir.GetNormalized(), forwardDir.GetNormalized());
		aRotation = Quaternion<T>(rotationMatrix).GetEulerAngles();

		aScale.x = rightDir.Length();
		aScale.y = upDir.Length();
		aScale.z = forwardDir.Length();
	}

	template<typename T>
	inline void Matrix4x4<T>::Decompose(Vector3<T>& aPosition, Quaternion<T>& aOrientaion, Vector3<T>& aScale) const
	{
		aPosition = { m41, m42, m43 };

		const Vector3<T> rightDir = { m11, m12, m13 };
		const Vector3<T> upDir = { m21, m22, m23 };
		const Vector3<T> forwardDir = { m31, m32, m33 };

		const Matrix3x3<T> rotationMatrix = Matrix4x4<T>::CreateRotationMatrix(rightDir.GetNormalized(), upDir.GetNormalized(), forwardDir.GetNormalized());
		aOrientaion = Quaternion<T>(rotationMatrix);

		aScale.x = rightDir.Length();
		aScale.y = upDir.Length();
		aScale.z = forwardDir.Length();
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			T(1), T(0), T(0), T(0),
			T(0), c, s, T(0),
			T(0), -s, c, T(0),
			T(0), T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			c, T(0), -s, T(0),
			T(0), T(1), T(0), T(0),
			s, T(0), c, T(0),
			T(0), T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			c, s, T(0), T(0),
			-s, c, T(0), T(0),
			T(0), T(0), T(1),T(0),
			T(0), T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::LookAt(const Vector3<T>& aPosition, const Vector3<T>& aTarget, const Vector3<T>& aUp)
	{
		const Vector3<T> forwardDir = (aTarget - aPosition).GetNormalized();

		if (forwardDir == aUp) return Matrix4x4<T>::Identity;

		const Vector3<T> rightDir = aUp.Cross(forwardDir).GetNormalized();
		const Vector3<T> upDir = forwardDir.Cross(rightDir).GetNormalized();

		return CreateRotationMatrix(rightDir, upDir, forwardDir);
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreatePerspectiveProjection(float aFOV, float aNearPlane, float aFarPlane, float aAspectRatio)
	{
		const float zoomX = 1.0f / tanf(aFOV * 0.5f);
		const float zoomY = aAspectRatio * zoomX;
		const float depth0 = aFarPlane / (aFarPlane - aNearPlane);
		const float depth1 = -aNearPlane * aFarPlane / (aFarPlane - aNearPlane);

		Matrix4x4<T> result = Matrix4x4<T>::Zero;
		result.m11 = zoomX;
		result.m22 = zoomY;
		result.m33 = depth0;
		result.m34 = 1.0f;
		result.m43 = depth1;
		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateOrthographicProjection(float aLeftPlane, float aRightPlane, float aBottomPlane, float aTopPlane, float aNearPlane, float aFarPlane)
	{
		const float reciprocalWidth = 1.0f / (aRightPlane - aLeftPlane);
		const float reciprocalHeight = 1.0f / (aTopPlane - aBottomPlane);
		const float fRange = 1.0f / (aFarPlane - aNearPlane);

		Matrix4x4<T> result = Matrix4x4<T>::Zero;
		result.m11 = reciprocalWidth + reciprocalWidth;
		result.m22 = reciprocalHeight + reciprocalHeight;
		result.m33 = fRange;
		result.m41 = -(aLeftPlane + aRightPlane) * reciprocalWidth;
		result.m42 = -(aTopPlane + aBottomPlane) * reciprocalHeight;
		result.m43 = -fRange * aNearPlane;
		result.m44 = 1.0f;
		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(const Vector3<T>& aScale)
	{
		Matrix4x4<T> result;
		result.myData[0] = aScale.x;
		result.myData[5] = aScale.y;
		result.myData[10] = aScale.z;
		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationMatrix(const Vector3<T>& aRight, const Vector3<T>& aUp, const Vector3<T>& aForward)
	{
		Matrix4x4<T> result;
		result[0] = aRight.x;
		result[1] = aRight.y;
		result[2] = aRight.z;

		result[4] = aUp.x;
		result[5] = aUp.y;
		result[6] = aUp.z;

		result[8] = aForward.x;
		result[9] = aForward.y;
		result[10] = aForward.z;
		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationMatrix(const Vector3<T>& aRotation)
	{
		return Quaternion<T>(aRotation).GetRotationMatrix4x4f();
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslationMatrix(const Vector3<T>& aTranslation)
	{
		Matrix4x4<T> result;
		result.myData[12] = aTranslation.x;
		result.myData[13] = aTranslation.y;
		result.myData[14] = aTranslation.z;
		return result;
	}


	template<typename T>
	const Matrix4x4<T> Matrix4x4<T>::Zero({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
	template<typename T>
	const Matrix4x4<T> Matrix4x4<T>::Identity({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });

	typedef Matrix4x4<float> Matrix4x4f;
}