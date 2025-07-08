#pragma once
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Color.h>

namespace Epoch::DataTypes
{
	struct Vertex
	{
		CU::Vector3f Position;
		CU::Vector3f Normal;
		CU::Vector3f Tangent;
		CU::Vector2f UV;
		CU::Vector3f Color;

		Vertex() :
			Position(CU::Vector4f::Zero),
			Normal(CU::Vector3f::Zero),
			Tangent(CU::Vector3f::Zero),
			UV(CU::Vector2f::Zero),
			Color(CU::Vector3f::One) {}

		Vertex(float x, float y, float z,
			   float nx, float ny, float nz,
			   float tx, float ty, float tz,
			   float u = 0.0f, float v = 0.0f,
			   float r = 1.0f, float g = 1.0f, float b = 1.0f)
		{
			Position = { x, y, z };
			Normal = { nx, ny, nz };
			Tangent = { tx, ty, tz };
			UV = { u, v };
			Color = { r, g, b };
		}
	};
}
