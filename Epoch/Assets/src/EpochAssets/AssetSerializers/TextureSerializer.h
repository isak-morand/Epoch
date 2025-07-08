#pragma once
#include "AssetSerializer.h"
#include <EpochDataTypes/TextureData.h>

namespace Epoch::Assets
{
	class TextureSerializer : public AssetSerializer
	{
	public:
		bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const override;

	private:
		bool FillTextureData(const std::filesystem::path& aFilePath, DataTypes::TextureData& outTextureData, const TextureImportSettings& aImportSettings) const;
	};
}
