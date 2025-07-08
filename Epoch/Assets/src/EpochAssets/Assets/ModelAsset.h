#pragma once
#include <EpochDataTypes/ModelData.h>
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	class ModelAsset : public Asset
	{
	public:
		ModelAsset() = delete;
		ModelAsset(AssetHandle aHandle) : Asset(aHandle) {}

		static AssetType GetStaticType() { return AssetType::Model; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const DataTypes::ModelData& GetData() const { return myData; }

	private:
		void SetData(DataTypes::ModelData&& aData) { myData = std::move(aData); }

	private:
		DataTypes::ModelData myData;

		friend class ModelSerializer;
	};
}
