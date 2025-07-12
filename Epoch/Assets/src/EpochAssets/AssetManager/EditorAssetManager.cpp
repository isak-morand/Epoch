#include "EditorAssetManager.h"
#include <CommonUtilities/StringUtils.h>
#include <EpochCore/JobSystem.h>
#include "EpochAssets/AssetExtensions.h"
#include "EpochAssets/AssetImporter.h"
#include "EpochAssets/Metadata/AssetMetadataSerializer.h"

namespace Epoch::Assets
{
	static AssetMetadata staticNullMetadata;

	EditorAssetManager::EditorAssetManager() = default;
	EditorAssetManager::~EditorAssetManager() = default;

	void EditorAssetManager::Init(const std::filesystem::path& aAssetDirectory)
	{
		myAssetDirectory = aAssetDirectory;

		AssetMetadataSerializer::Init();
		AssetImporter::Init();

		RegisterAssets();
	}

	/*void EditorAssetManager::FlushLoadedAssets()
	{
		std::lock_guard<std::mutex> lock(myAssetMutex);

		for (auto it = myLoadingAssets.begin(); it != myLoadingAssets.end();)
		{
			auto& future = it->second;
			if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
			{
				++it;
				continue;
			}

			std::shared_ptr<Asset> asset = future.get();

			if (asset)
			{
				myLoadedAssets[it->first] = asset;
				const AssetMetadata& metadata = GetMetadata(it->first);
				AssetMetadataSerializer::Serialize(GetMetaFilePath(it->first), metadata);
			}

			it = myLoadingAssets.erase(it);
		}
	}*/

	std::shared_ptr<Asset> EditorAssetManager::GetAsset(AssetHandle aHandle)
	{
		if (auto it = myMemoryAssets.find(aHandle); it != myMemoryAssets.end())
		{
			return it->second;
		}

		if (auto it = myLoadedAssets.find(aHandle); it != myLoadedAssets.end())
		{
			return it->second;
		}

		const AssetMetadata& metadata = GetMetadata(aHandle);
		if (!metadata.IsValid())
		{
			return nullptr;
		}

		auto parentIt = myAssetParents.find(aHandle);
		if (parentIt != myAssetParents.end())
		{
			if (!GetAsset(parentIt->second)) return nullptr;

			auto subIt = myMemoryAssets.find(aHandle);
			return subIt != myMemoryAssets.end() ? subIt->second : nullptr;
		}
		else
		{
			if (myAssetParents.contains(aHandle))
			{
				myAssetParents.erase(aHandle);
			}

			if (myAssetSubAssets.contains(aHandle))
			{
				myAssetSubAssets.erase(aHandle);
			}

			std::shared_ptr<Asset> asset;
			if (AssetImporter::TryLoadData(metadata, asset))
			{
				myLoadedAssets[aHandle] = asset;
				AssetMetadataSerializer::Serialize(GetMetaFilePath(aHandle), metadata);
			}
			return asset;
		}
		return nullptr;
	}

	/*std::shared_ptr<Asset> EditorAssetManager::GetAssetAsync(AssetHandle aHandle)
	{
		std::lock_guard<std::mutex> lock(myAssetMutex);

		if (auto it = myMemoryAssets.find(aHandle); it != myMemoryAssets.end())
		{
			return it->second;
		}

		if (auto it = myLoadedAssets.find(aHandle); it != myLoadedAssets.end())
		{
			return it->second;
		}

		const AssetMetadata& metadata = GetMetadata(aHandle);
		if (!metadata.IsValid())
		{
			return nullptr;
		}

		auto parentIt = myAssetParents.find(aHandle);
		if (parentIt != myAssetParents.end())
		{
			AssetHandle parent = parentIt->second;
			std::shared_ptr<Asset> parentAsset = GetAssetAsync(parent);
			if (!parentAsset)
			{
				return nullptr; // Parent still loading
			}

			// Parent loaded; sub-asset should now be registered
			auto subIt = myMemoryAssets.find(aHandle);
			return subIt != myMemoryAssets.end() ? subIt->second : nullptr;
		}

		// Already loading?
		if (myLoadingAssets.contains(aHandle))
		{
			return nullptr;
		}
		else
		{
			if (myAssetParents.contains(aHandle))
			{
				myAssetParents.erase(aHandle);
			}

			if (myAssetSubAssets.contains(aHandle))
			{
				myAssetSubAssets.erase(aHandle);
			}
		}

		// Submit loading job
		auto& js = Core::JobSystem::GetJobSystem();
		std::shared_future<std::shared_ptr<Asset>> future =
			js.SubmitJob("LoadAsset_" + std::to_string(aHandle), [=]() -> std::shared_ptr<Asset>
						 {
							 std::shared_ptr<Asset> loaded;
							 if (AssetImporter::TryLoadData(metadata, loaded))
							 {
								 return loaded;
							 }
							 return nullptr;
						 });

		myLoadingAssets[aHandle] = future;
		return nullptr;
	}*/

	void EditorAssetManager::AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, std::string_view aName)
	{
		AssetMetadata metadata;
		metadata.Handle = aAsset->GetHandle();
		metadata.Type = aAsset->GetAssetType();
		metadata.IsMemoryAsset = true;

		if (!aName.empty())
		{
			metadata.FilePath = aName;
		}

		myAssetRegistry[metadata.Handle] = metadata;
		myMemoryAssets[metadata.Handle] = std::move(aAsset);
	}

	void EditorAssetManager::AddSubAsset(AssetHandle aParentAsset, std::shared_ptr<Asset> aAsset, std::string_view aName)
	{
		AddMemoryOnlyAsset(aAsset, aName);
		RegisterSubAsset(aParentAsset, aAsset->GetHandle());
	}

	void EditorAssetManager::ReloadAsset(AssetHandle aHandle)
	{
		auto& metadata = GetMetadata(aHandle);
		EPOCH_ASSERT(metadata.IsValid(), "Trying to reload invalid asset!");

		const auto& subAssets = GetSubAssets(aHandle);
		for (auto subAsset : subAssets)
		{
			RemoveAsset(subAsset);
		}
		if (myLoadedAssets.contains(aHandle))
		{
			myLoadedAssets.erase(aHandle);
		}

		std::shared_ptr<Asset> asset;
		if (AssetImporter::TryLoadData(metadata, asset))
		{
			myLoadedAssets[aHandle] = asset;
		}
	}

	void EditorAssetManager::RemoveAsset(AssetHandle aHandle)
	{
		const auto& subAssets = GetSubAssets(aHandle);
		for (auto subAsset : subAssets)
		{
			RemoveAsset(subAsset);
		}

		if (myAssetParents.contains(aHandle))
		{
			myAssetParents.erase(aHandle);
		}

		if (myAssetSubAssets.contains(aHandle))
		{
			myAssetSubAssets.erase(aHandle);
		}

		if (myLoadedAssets.contains(aHandle))
		{
			myLoadedAssets.erase(aHandle);
		}

		if (myMemoryAssets.contains(aHandle))
		{
			myMemoryAssets.erase(aHandle);
		}

		if (myAssetRegistry.contains(aHandle))
		{
			myAssetRegistry.erase(aHandle);
		}
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& aFilepath)
	{
		std::filesystem::path path = GetRelativePath(aFilepath);

		if (const auto& metadata = GetMetadata(path); metadata.IsValid())
		{
			return metadata.Handle;
		}

		AssetMetadata metadata;

		std::filesystem::path metaPath = GetMetaFilePath(path);
		if (std::filesystem::exists(metaPath))
		{
			metadata = AssetMetadataSerializer::Deserialize(metaPath);
			metadata.FilePath = path;
		}
		else
		{
			AssetType type = GetAssetTypeFromExtension(path.extension().string());
			if (type == AssetType::None)
			{
				return 0;
			}

			metadata.Handle = AssetHandle();
			metadata.FilePath = path;
			metadata.Type = type;

			metadata.ImportSettings = ImportSettingsFactory::CreateDefault(type);

			AssetMetadataSerializer::Serialize(metaPath, metadata);
		}

		myAssetRegistry.insert_or_assign(metadata.Handle, metadata);

		return metadata.Handle;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle aHandle)
	{
		if (auto it = myAssetRegistry.find(aHandle); it != myAssetRegistry.end())
		{
			return it->second;
		}

		return staticNullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const std::filesystem::path& aFilepath)
	{
		const auto relativePath = GetRelativePath(aFilepath);

		for (auto& [handle, metadata] : myAssetRegistry)
		{
			if (metadata.FilePath == relativePath)
			{
				return metadata;
			}
		}

		return staticNullMetadata;
	}

	std::set<AssetHandle> EditorAssetManager::GetSubAssets(AssetHandle aHandle) const
	{
		if (auto it = myAssetSubAssets.find(aHandle); it != myAssetSubAssets.end())
		{
			return it->second;
		}
		return {};
	}

	AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string& aExtension)
	{
		std::string ext = CU::ToLower(aExtension);
		if (auto it = staticAssetExtensionMap.find(ext); it != staticAssetExtensionMap.end())
		{
			return it->second;
		}

		return AssetType::None;
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(AssetHandle aHandle)
	{
		return myAssetDirectory / GetMetadata(aHandle).FilePath;
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(const std::filesystem::path& aRelativePath)
	{
		return myAssetDirectory / aRelativePath;
	}

	std::filesystem::path EditorAssetManager::GetRelativePath(const std::filesystem::path& aFilepath)
	{
		std::filesystem::path relativePath = aFilepath.lexically_normal();
		std::string temp = aFilepath.string();
		if (temp.find(myAssetDirectory.string()) != std::string::npos)
		{
			relativePath = std::filesystem::relative(aFilepath, myAssetDirectory);
			if (relativePath.empty())
			{
				relativePath = aFilepath.lexically_normal();
			}
		}
		return relativePath;
	}

	std::filesystem::path EditorAssetManager::GetMetaFilePath(AssetHandle aHandle)
	{
		return (GetFileSystemPath(aHandle) += ".meta");
	}

	std::filesystem::path EditorAssetManager::GetMetaFilePath(const std::filesystem::path& aRelativePath)
	{
		return (GetFileSystemPath(aRelativePath) += ".meta");
	}

	void EditorAssetManager::RegisterAssets()
	{
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(myAssetDirectory))
		{
			if (entry.is_directory()) continue;

			ImportAsset(entry.path());
		}
	}

	void EditorAssetManager::RegisterMetadata(const AssetMetadata& aMetadata)
	{
		myAssetRegistry.insert_or_assign(aMetadata.Handle, aMetadata);
	}

	void EditorAssetManager::RegisterSubAsset(AssetHandle aAsset, AssetHandle aSubAsset)
	{
		myAssetSubAssets[aAsset].insert(aSubAsset);
		myAssetParents[aSubAsset] = aAsset;
	}
}
