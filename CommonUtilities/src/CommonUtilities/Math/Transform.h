#pragma once
#include "Vector/Vector.h"
#include "Matrix/Matrix.h"
#include "Quaternion.hpp"
#include "CommonMath.hpp"

namespace CU
{
	class Transform
	{
	public:
		Transform(Vector3f aTranslation = Vector3f::Zero, Vector3f aRotation = Vector3f::Zero, Vector3f aScale = Vector3f::One);
		Transform(Vector3f aTranslation, Quatf aRotation, Vector3f aScale = Vector3f::One);
		Transform(const Matrix4x4f& aMatrix);
		~Transform() = default;

		const Vector3f& GetTranslation() const { return myTranslation; }
		Vector3f& GetTranslation() { return myTranslation; }
		Vector4f GetTranslationVec4() const { return Vector4f(myTranslation, 1.0f); }
		const Vector3f& GetRotation() const { return myRotationEuler; }
		Vector3f& GetRotation() { return myRotationEuler; }
		const Quatf& GetRotationQuat() const { return myRotationQuat; }
		const Vector3f& GetScale() const { return myScale; }
		Vector3f& GetScale() { return myScale; }

		Vector3f GetRight() const { return myRotationQuat.GetRight(); }
		Vector3f GetUp() const { return myRotationQuat.GetUp(); }
		Vector3f GetForward() const { return myRotationQuat.GetForward(); }

		const Matrix4x4f& GetMatrix();
		void SetTransform(const Matrix4x4f& aMatrix);
		void SetTransform(const Transform& aTransform);

		void SetTranslation(const Vector3f& aTranslation){ myTranslation = aTranslation; myIsDirty = true; }
		void SetTranslation(float aX, float aY, float aZ) { myTranslation = { aX, aY, aZ }; myIsDirty = true; }
		void SetRotation(const Vector3f& aRotation) { myRotationEuler = aRotation; myRotationQuat = Quatf(myRotationEuler); myIsDirty = true; }
		void SetRotation(float aX, float aY, float aZ) { myRotationEuler = { aX, aY, aZ }; myRotationQuat = Quatf(myRotationEuler); myIsDirty = true; }
		void SetRotation(const CU::Quatf& aOrientation) { myRotationQuat = aOrientation; myRotationEuler = aOrientation.GetEulerAngles(); myIsDirty = true; }
		void SetScale(const Vector3f& aScale) { myScale = aScale; myIsDirty = true; }
		void SetScale(float aX, float aY, float aZ) { myScale = { aX, aY, aZ }; myIsDirty = true; }

		void Translate(const Vector3f& aTranslation) { myTranslation += aTranslation; myIsDirty = true; }
		void Rotate(const Vector3f& aRotation) { myRotationEuler += aRotation; myRotationQuat = Quatf(myRotationEuler); myIsDirty = true; }
		void Scale(const Vector3f& aScale) { myScale += aScale; myIsDirty = true; }

		void RotateAround(const Vector3f& aPoint, const Vector3f& aAxis, float aAngle);
		void LookAt(const Vector3f& aTarget, const Vector3f& aUp = Vector3f::Up);

	private:
		Vector3f myTranslation;
		Vector3f myRotationEuler;
		Quatf myRotationQuat;
		Vector3f myScale;

		bool myIsDirty = false;
		Matrix4x4f myMatrix;
	};
}
