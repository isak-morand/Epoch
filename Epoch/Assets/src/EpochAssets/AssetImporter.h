#pragma once
#include <memory>
#include <unordered_map>
#include "Asset.h"
#include "Metadata/AssetMetadata.h"
#include "AssetSerializers/AssetSerializer.h"

namespace Epoch::Assets
{
	class AssetImporter
	{
	public:
		static void Init();
		static void RegisterSerializer(AssetType aType, std::unique_ptr<AssetSerializer> aSerializer);

		static bool TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset);

	private:
		inline static std::unordered_map<AssetType, std::unique_ptr<AssetSerializer>> staticSerializers;
	};
}
