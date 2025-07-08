#include "AssetImporter.h"

#include "AssetSerializers/TextureSerializer.h"
#include "AssetSerializers/ModelSerializer.h"
#include "AssetSerializers/SceneAssetSerializer.h"

namespace Epoch::Assets
{
	void AssetImporter::Init()
	{
		staticSerializers[AssetType::Texture]	= std::make_unique<TextureSerializer>();
		staticSerializers[AssetType::Model]		= std::make_unique<ModelSerializer>();
		staticSerializers[AssetType::Scene]		= std::make_unique<SceneAssetSerializer>();
	}

	void AssetImporter::RegisterSerializer(AssetType aType, std::unique_ptr<AssetSerializer> aSerializer)
	{
		staticSerializers[aType] = std::move(aSerializer);
	}

	bool AssetImporter::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset)
	{
		if (staticSerializers.find(aMetadata.Type) == staticSerializers.end())
		{
			LOG_WARNING("There's currently no importer for assets of type '{}'", Utils::AssetTypeToString(aMetadata.Type));
			return false;
		}

		return staticSerializers[aMetadata.Type]->TryLoadData(aMetadata, outAsset);
	}
}
