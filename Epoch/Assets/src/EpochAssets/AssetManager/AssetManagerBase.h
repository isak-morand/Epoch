#pragma once
#include <memory>
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	using AssetMap = std::unordered_map<AssetHandle, std::shared_ptr<Asset>>;

	class AssetManagerBase
	{
	public:
		AssetManagerBase() = default;
		virtual ~AssetManagerBase() = default;

		virtual std::shared_ptr<Asset> GetAsset(AssetHandle aHandle) = 0;

		virtual void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, std::string_view aName = {}) = 0;

		virtual void ReloadAsset(AssetHandle aHandle) = 0;
		virtual void RemoveAsset(AssetHandle aHandle) = 0;
	};
}
