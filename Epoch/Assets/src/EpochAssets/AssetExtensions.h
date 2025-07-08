#pragma once
#include <unordered_map>
#include "AssetTypes.h"

namespace Epoch::Assets
{
	inline static std::unordered_map<std::string, AssetType> staticAssetExtensionMap =
	{
		// Epoch types
		{ ".epoch",		AssetType::Scene },

		// Mesh/Animation source
		{ ".fbx",		AssetType::Model },
		{ ".gltf",		AssetType::Model },
		{ ".glb",		AssetType::Model },
		{ ".obj",		AssetType::Model },

		// Texture
		{ ".png",		AssetType::Texture },
		{ ".jpg",		AssetType::Texture },
		{ ".jpeg",		AssetType::Texture },
	};
}