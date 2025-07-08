#pragma once
#include <filesystem>
#include <unordered_map>
#include <set>
#include "AssetManagerBase.h"
#include "EpochAssets/Metadata/AssetMetadata.h"

namespace Epoch::Assets
{
	class EditorAssetManager : public AssetManagerBase
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override;

		void Init(const std::filesystem::path& aAssetDirectory);

		std::shared_ptr<Asset> GetAsset(AssetHandle aHandle) override;

		void AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, std::string_view aName = {}) override;
		void AddSubAsset(AssetHandle aParentAsset, std::shared_ptr<Asset> aAsset, std::string_view aName = {});

		void ReloadAsset(AssetHandle aHandle) override;
		void RemoveAsset(AssetHandle aHandle) override;

		AssetHandle ImportAsset(const std::filesystem::path& aFilepath);

		const AssetMetadata& GetMetadata(AssetHandle aHandle);
		const AssetMetadata& GetMetadata(const std::filesystem::path& aFilepath);
		
		std::set<AssetHandle> GetSubAssets(AssetHandle aHandle) const;

		AssetType GetAssetTypeFromExtension(const std::string& aExtension);

		std::filesystem::path GetFileSystemPath(AssetHandle aHandle);
		std::filesystem::path GetFileSystemPath(const std::filesystem::path& aRelativePath);
		std::filesystem::path GetRelativePath(const std::filesystem::path& aFilepath);

		std::filesystem::path GetMetaFilePath(AssetHandle aHandle);
		std::filesystem::path GetMetaFilePath(const std::filesystem::path& aRelativePath);

	private:
		void RegisterAssets();
		void RegisterMetadata(const AssetMetadata& aMetadata);
		void RegisterDependency(AssetHandle aAsset, AssetHandle aSubAsset);

	private:
		std::filesystem::path myAssetDirectory;

		std::unordered_map<AssetHandle, AssetMetadata> myAssetRegistry;
		std::unordered_map<AssetHandle, std::set<AssetHandle>> myAssetSubAssets; //ParentAsset -> SubAssets
		std::unordered_map<AssetHandle, AssetHandle> myAssetDependencies; //SubAsset -> ParentAsset

		AssetMap myLoadedAssets;
		AssetMap myMemoryAssets;

		friend class AssetMetadataSerializer;
	};
}
