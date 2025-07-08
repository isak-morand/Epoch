#pragma once
#include <vector>
#include <EpochDataTypes/MeshData.h>
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	class MeshAsset : public Asset
	{
	public:
		MeshAsset() = delete;
		MeshAsset(AssetHandle aHandle) : Asset(aHandle) {}

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const DataTypes::MeshData& GetData() const { return myData; }

	private:
		void SetData(DataTypes::MeshData&& aData) { myData = std::move(aData); }

	private:
		DataTypes::MeshData myData;

		friend class MeshImporter;
	};
}
