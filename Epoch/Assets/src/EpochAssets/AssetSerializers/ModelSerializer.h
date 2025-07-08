#pragma once
#include "AssetSerializer.h"
#include <EpochDataTypes/ModelData.h>

namespace Epoch::Assets
{
	class ModelSerializer : public AssetSerializer
	{
	public:
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const override;

	private:
		bool FillModelData(const AssetMetadata& aMetadata, DataTypes::ModelData& outModelData, const ModelImportSettings& aImportSettings) const;
	};
}
