#include "SceneAssetSerializer.h"
#include "EpochAssets/Assets/SceneAsset.h"

namespace Epoch::Assets
{
	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const
	{
		auto sceneAsset = std::make_shared<SceneAsset>(aMetadata.Handle);
		outAsset = sceneAsset;

		return true;
	}
}
