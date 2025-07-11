#pragma once
#include <vector>
#include <EpochDataTypes/SkeletonData.h>
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	class SkeletonAsset : public Asset
	{
	public:
		SkeletonAsset() = delete;
		SkeletonAsset(AssetHandle aHandle) : Asset(aHandle) {}

		static AssetType GetStaticType() { return AssetType::Skeleton; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const DataTypes::Skeleton& GetData() const { return myData; }

	private:
		void SetData(DataTypes::Skeleton&& aData) { myData = std::move(aData); }

	private:
		DataTypes::Skeleton myData;

		friend class MeshImporter;
	};
}
