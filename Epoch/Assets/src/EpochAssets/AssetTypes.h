#pragma once
#include <string_view>
#include <EpochCore/Assert.h>

namespace Epoch::Assets
{
	enum class AssetType : uint16_t
	{
		None,
		Texture,
		Model,
		Mesh,
		Skeleton,
		Material,
		Scene
	};

	namespace Utils
	{
		inline AssetType AssetTypeFromString(std::string_view aAssetType)
		{
			if (aAssetType == "None")		return AssetType::None;
			if (aAssetType == "Texture")	return AssetType::Texture;
			if (aAssetType == "Model")		return AssetType::Model;
			if (aAssetType == "Mesh")		return AssetType::Mesh;
			if (aAssetType == "Material")	return AssetType::Material;
			if (aAssetType == "Scene")		return AssetType::Scene;

			EPOCH_ASSERT(false, "Unknown Asset Type");
			return AssetType::None;
		}

		inline const char* AssetTypeToString(AssetType aAssetType)
		{
			switch (aAssetType)
			{
				case AssetType::None:		return "None";
				case AssetType::Texture:	return "Texture";
				case AssetType::Model:		return "Model";
				case AssetType::Mesh:		return "Mesh";
				case AssetType::Material:	return "Material";
				case AssetType::Scene:		return "Scene";
			}

			EPOCH_ASSERT(false, "Unknown Asset Type");
			return "None";
		}
	}
}
