#pragma once
#include <filesystem>
#include "MeshImporter.h"

namespace Epoch::Assets
{
	class AssimpMeshImporter : public MeshImporter
	{
	public:
		AssimpMeshImporter() = default;
		~AssimpMeshImporter() override = default;

		bool ImportMesh(const AssetMetadata& aMetadata, DataTypes::ModelData& outModelData, const ModelImportSettings& aImportSettings) override;

	private:

	};
}
