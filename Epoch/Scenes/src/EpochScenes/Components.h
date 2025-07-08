#pragma once
#include <string>
#include <entt/entt.hpp>
#include <CommonUtilities/Math/Transform.h>
#include <EpochCore/UUID.h>

#include <EpochAssets/Assets/MeshAsset.h>

namespace Epoch::Assets
{
	class MeshAsset;
}

namespace Epoch::Scenes
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(UUID aID) : ID(aID) {}
	};

	struct NameComponent
	{
		std::string Name;

		NameComponent() = default;
		NameComponent(std::string_view aName) : Name(std::string(aName)) {}
		NameComponent(const std::string& aName) : Name(aName) {}
	};

	struct TransformComponent
	{
		CU::Transform LocalTransform;
		CU::Matrix4x4f WorldMatrix;

		bool WorldDirty = true;

		TransformComponent() = default;
		TransformComponent(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation = CU::Vector3f::Zero, const CU::Vector3f& aScale = CU::Vector3f::One) : LocalTransform(aPosition, aRotation, aScale) {}
	};

	struct ParentComponent
	{
		entt::entity Parent = entt::null;

		ParentComponent() = default;
		ParentComponent(entt::entity aParent) : Parent(aParent) {}
	};

	struct ChildrenComponent
	{
		std::vector<entt::entity> Children;
	};

	struct MeshRendererComponent
	{
		TypedAssetHandle<Assets::MeshAsset> Mesh;

		MeshRendererComponent() = default;
		MeshRendererComponent(AssetHandle aMesh) : Mesh(aMesh) {}
	};
}
