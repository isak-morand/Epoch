#pragma once
#include <filesystem>
#include <EpochDataTypes/ModelData.h>
#include <EpochDataTypes/MeshData.h>
#include <EpochDataTypes/SkeletonData.h>
#include "EpochAssets/Metadata/AssetMetadata.h"

namespace Epoch::Assets
{
	class MeshAsset;

	class MeshImporter
	{
	public:
		MeshImporter() = default;
		virtual ~MeshImporter() = default;

		virtual bool ImportMesh(const AssetMetadata& aMetadata, DataTypes::ModelData& outModelData, const ModelImportSettings& aImportSettings) = 0;

	protected:
		void SetMeshData(std::shared_ptr<MeshAsset>& outMeshAsset, DataTypes::MeshData& aMeshData) const;
	};
}
