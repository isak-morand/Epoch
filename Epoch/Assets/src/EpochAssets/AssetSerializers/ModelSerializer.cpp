#include "ModelSerializer.h"
#include "EpochAssets/AssetManager.h"
#include "EpochAssets/Assets/ModelAsset.h"
#include "EpochAssets/Metadata/AssetMetadataSerializer.h"
#include "MeshImporters/AssimpMeshImporter.h"

namespace Epoch::Assets
{
	bool ModelSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const
	{
		const ModelImportSettings& importSettings = aMetadata.GetImportSettings<ModelImportSettings>();

		DataTypes::ModelData modelData;
		if (!FillModelData(aMetadata, modelData, importSettings))
		{
			return false;
		}

		if (!AssetManager::GetEditorAssetManager()->GetSubAssets(aMetadata.Handle).empty())
		{
			AssetMetadataSerializer::Serialize(AssetManager::GetEditorAssetManager()->GetMetaFilePath(aMetadata.FilePath), aMetadata);
		}

		auto modelAsset = std::make_shared<ModelAsset>(aMetadata.Handle);
		modelAsset->SetData(std::move(modelData));
		outAsset = modelAsset;

		return true;
	}

	bool ModelSerializer::FillModelData(const AssetMetadata& aMetadata, DataTypes::ModelData& outModelData, const ModelImportSettings& aImportSettings) const
	{
		auto extension = aMetadata.FilePath.extension().string();

		if (extension == ".fbx")
		{
			//EPOCH_ASSERT(false, "FBX importer not implemented!");
			//return false;
		}
		else if (extension == ".gltf" || extension == ".glb")
		{
			//EPOCH_ASSERT(false, "GLTF/GLB importer not implemented!");
			//return false;
		}

		AssimpMeshImporter importer;
		return importer.ImportMesh(aMetadata, outModelData, aImportSettings);
	}
}
