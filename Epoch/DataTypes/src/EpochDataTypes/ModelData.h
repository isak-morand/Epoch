#pragma once
#include <vector>
#include <string>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>
#include <EpochCore/UUID.h>

namespace Epoch::DataTypes
{
	struct ModelData
	{
		std::vector<UUID> MeshAssets;
		UUID SkeletonAsset = 0;

		struct Node
		{
			std::string Name;
			CU::Matrix4x4f LocalTransform;

			uint32_t Parent = UINT32_MAX;
			std::vector<uint32_t> Children;

			uint32_t MeshIndex = UINT32_MAX;

			inline bool IsRoot() const { return Parent == UINT32_MAX; }
		};

		std::vector<Node> Hierarchy;

		bool IsValid() const { return !Hierarchy.empty(); } //TODO: This is only the case if animations aren't supported
	};
}
