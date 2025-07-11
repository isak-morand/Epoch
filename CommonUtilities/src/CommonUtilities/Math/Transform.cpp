#include "Transform.h"

namespace CU
{
	Transform::Transform(Vector3f aTranslation, Vector3f aRotation, Vector3f aScale) : myTranslation(aTranslation), myRotationEuler(aRotation), myRotationQuat(aRotation), myScale(aScale), myIsDirty(true) {}

	Transform::Transform(Vector3f aTranslation, Quatf aRotation, Vector3f aScale) : myTranslation(aTranslation), myRotationQuat(aRotation), myScale(aScale), myIsDirty(true)
	{
		myRotationEuler = myRotationQuat.GetEulerAngles();
	}

	Transform::Transform(const Matrix4x4f& aMatrix) : myMatrix(aMatrix)
	{
		aMatrix.Decompose(myTranslation, myRotationQuat, myScale);
		myRotationEuler = myRotationQuat.GetEulerAngles();
		myIsDirty = false;
	}

	const Matrix4x4f& Transform::GetMatrix()
	{
		if (myIsDirty)
		{
			const Matrix4x4f scaleMatrix = Matrix4x4f::CreateScaleMatrix(myScale);
			const Matrix4x4f rotationMatrix = myRotationQuat.GetRotationMatrix4x4f();
			const Matrix4x4f translationMatrix = Matrix4x4f::CreateTranslationMatrix(myTranslation);

			myMatrix = scaleMatrix * rotationMatrix * translationMatrix;
			myIsDirty = false;
		}

		return myMatrix;
	}

	void Transform::SetTransform(const Matrix4x4f& aMatrix)
	{
		aMatrix.Decompose(myTranslation, myRotationQuat, myScale);
		myRotationEuler = myRotationQuat.GetEulerAngles();
		myMatrix = aMatrix;
		myIsDirty = false;
	}

	void Transform::SetTransform(const Transform& aTransform)
	{
		*this = aTransform;
		//myTranslation = aTransform.myTranslation;
		//myRotationEuler = aTransform.myRotationEuler;
		//myScale = aTransform.myScale;
		//myRotationQuat = aTransform.myRotationQuat;
		//myMatrix = aTransform.myMatrix;
		//myIsDirty = aTransform.myIsDirty;
	}

	void Transform::RotateAround(const Vector3f& aPoint, const Vector3f& aAxis, float aAngle)
	{
		const Quatf quat = Quatf(aAxis, aAngle);
		myRotationQuat *= quat;
		myRotationEuler += quat.GetEulerAngles();

		myTranslation = Quatf::RotateVectorByQuaternion((myTranslation - aPoint), quat) + aPoint;
		myIsDirty = true;
	}

	void Transform::LookAt(const Vector3f& aTarget, const Vector3f& aUp)
	{
		const Vector3f forwardDir = (aTarget - myTranslation).GetNormalized();

		if (forwardDir == aUp) return;

		const Vector3f rightDir = aUp.Cross(forwardDir).GetNormalized();
		const Vector3f upDir = forwardDir.Cross(rightDir).GetNormalized();

		const Matrix3x3f rotationMatrix = Matrix4x4f::CreateRotationMatrix(rightDir, upDir, forwardDir);
		myRotationQuat = Quatf(rotationMatrix);
		myRotationEuler = myRotationQuat.GetEulerAngles();
		myIsDirty = true;
	}
}
