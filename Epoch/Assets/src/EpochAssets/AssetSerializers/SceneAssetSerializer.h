#pragma once
#include "AssetSerializer.h"

namespace Epoch::Assets
{
	class SceneAssetSerializer : public AssetSerializer
	{
	public:
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const override;
	};
}
