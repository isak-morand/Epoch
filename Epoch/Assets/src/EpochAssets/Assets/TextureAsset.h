#pragma once
#include <EpochDataTypes/TextureData.h>
#include "EpochAssets/Asset.h"

namespace Epoch::Assets
{
	class TextureAsset : public Asset
	{
	public:
		TextureAsset() = delete;
		TextureAsset(AssetHandle aHandle) : Asset(aHandle) {}
		~TextureAsset() { myData.Data.Release(); }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		const DataTypes::TextureData& GetData() const { return myData; }

	private:
		void SetData(DataTypes::TextureData&& aData) { myData = std::move(aData); }

	private:
		DataTypes::TextureData myData;

		friend class TextureSerializer;
	};
}
