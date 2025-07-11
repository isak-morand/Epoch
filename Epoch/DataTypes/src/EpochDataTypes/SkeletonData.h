#pragma once
#include <vector>
#include <string>
#include <CommonUtilities/Math/Quaternion.hpp>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>

namespace Epoch::DataTypes
{
	struct BoneTransform { CU::Vector3f Translation; CU::Quatf Rotation; CU::Vector3f Scale; };

	struct Skeleton
	{
		std::vector<std::string> BoneNames;
		std::vector<uint32_t> ParentIndices;
		std::vector<BoneTransform> RestPose;
		std::vector<CU::Matrix4x4f> BoneToWorldPose;

		bool IsValid() const { return !BoneNames.empty(); }
	};
}
