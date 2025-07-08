#include "MeshImporter.h"
#include "EpochAssets/Assets/MeshAsset.h"


namespace Epoch::Assets
{
    void MeshImporter::SetMeshData(std::shared_ptr<MeshAsset>& outMeshAsset, DataTypes::MeshData& aMeshData) const
    {
        outMeshAsset->SetData(std::move(aMeshData));
    }
}
