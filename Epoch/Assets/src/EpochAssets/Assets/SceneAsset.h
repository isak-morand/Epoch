#pragma once
#include "EpochAssets/Asset.h"
#include <filesystem>

namespace Epoch::Assets
{
	class SceneAsset : public Asset
	{
	public:
		SceneAsset() = delete;
		SceneAsset(AssetHandle aHandle) : Asset(aHandle) {}

		static AssetType GetStaticType() { return AssetType::Scene; }
		AssetType GetAssetType() const override { return GetStaticType(); }
	};
}
