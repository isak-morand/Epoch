#pragma once
#include <memory>
#include "EpochAssets/Metadata/AssetMetadata.h"
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	class AssetSerializer
	{
	public:
		virtual bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const = 0;
	};
}
