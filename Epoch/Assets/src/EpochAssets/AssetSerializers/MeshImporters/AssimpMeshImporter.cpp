#include "AssimpMeshImporter.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <EpochDataTypes/ModelData.h>
#include <EpochDataTypes/MeshData.h>
#include <EpochCore/Hash.h>
#include "EpochAssets/Assets/MeshAsset.h"
#include "EpochAssets/AssetManager.h"

namespace Epoch::Assets
{
	static const uint32_t staticMeshImportFlags =
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_GenUVCoords |             // Convert UVs if required 
		//aiProcess_OptimizeGraph |
		aiProcess_FindInstances |
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights |        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
		aiProcess_ValidateDataStructure |   // Validation
		aiProcess_GlobalScale |				// e.g. convert cm to m for fbx import (and other formats where cm is native)
		aiProcess_ConvertToLeftHanded
		;

	namespace Utils
	{
		CU::Matrix4x4f FromAIMat4(const aiMatrix4x4& aMatrix)
		{
			return
			{
				aMatrix.a1, aMatrix.b1, aMatrix.c1, aMatrix.d1,
				aMatrix.a2, aMatrix.b2, aMatrix.c2, aMatrix.d2,
				aMatrix.a3, aMatrix.b3, aMatrix.c3, aMatrix.d3,
				aMatrix.a4, aMatrix.b4, aMatrix.c4, aMatrix.d4
			};
		}

		AssetHandle GenerateSubAssetHandle(AssetHandle aBaseHandle, std::string_view aSubType, uint32_t aIndex)
		{
			std::string full = std::to_string(aBaseHandle) + "|" + std::string(aSubType) + "|" + std::to_string(aIndex);
			return Hash::GenerateFNVHash(full);
		}
	}

	bool AssimpMeshImporter::ImportMesh(const AssetMetadata& aMetadata, DataTypes::ModelData& outModelData, const ModelImportSettings& aImportSettings)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 100.0f); // convert to cm

		uint32_t importFlags = staticMeshImportFlags;

		if (aImportSettings.FlattenHierarchy)
		{
			importFlags |= aiProcess_PreTransformVertices;
		}

		const auto filepath = AssetManager::GetEditorAssetManager()->GetFileSystemPath(aMetadata.FilePath).string();
		const aiScene* scene = importer.ReadFile(filepath, importFlags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("Failed to import model:{} {}", aMetadata.FilePath.stem().string(), importer.GetErrorString());
			return false;
		}

		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			DataTypes::MeshData meshData;

			for (uint32_t v = 0; v < mesh->mNumVertices; ++v)
			{
				auto& vertex = meshData.Vertices.emplace_back();
				vertex.Position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
				vertex.Normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
				if (mesh->HasTangentsAndBitangents()) vertex.Tangent = { mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z };
				if (mesh->HasTextureCoords(0)) vertex.UV = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
				if (mesh->HasVertexColors(0)) vertex.Color = { mesh->mColors[0][v].r, mesh->mColors[0][v].g, mesh->mColors[0][v].b };
			}

			uint32_t indexOffset = 0;
			for (uint32_t f = 0; f < mesh->mNumFaces; ++f)
			{
				EPOCH_ASSERT(mesh->mFaces[f].mNumIndices == 3, "A face must have 3 indices!");

				meshData.Indices.push_back(mesh->mFaces[f].mIndices[0]);
				meshData.Indices.push_back(mesh->mFaces[f].mIndices[1]);
				meshData.Indices.push_back(mesh->mFaces[f].mIndices[2]);
			}

			meshData.Submeshes.push_back({ 0, (uint32_t)meshData.Indices.size() });

			if (meshData.IsValid())
			{
				UUID meshUUID = Utils::GenerateSubAssetHandle(aMetadata.Handle, "Mesh", i);
				auto meshAsset = std::make_shared<MeshAsset>(meshUUID);
				SetMeshData(meshAsset, meshData);

				AssetManager::GetEditorAssetManager()->AddSubAsset(aMetadata.Handle, meshAsset, mesh->mName.C_Str());

				outModelData.MeshAssets.push_back(meshUUID);
			}
		}

		std::function<void(aiNode*, int)> TraverseNode;
		TraverseNode = [&](aiNode* aNode, int aParentIndex)
			{
				DataTypes::ModelData::Node ourNode;
				ourNode.Name = aParentIndex == -1 ? aMetadata.FilePath.stem().string() : aNode->mName.C_Str();

				ourNode.LocalTransform = Utils::FromAIMat4(aNode->mTransformation);

				for (unsigned i = 0; i < aNode->mNumMeshes; ++i)
				{
					ourNode.MeshIndex = (uint32_t)aNode->mMeshes[i];
					break;
				}

				int thisIndex = static_cast<int>(outModelData.Hierarchy.size());
				outModelData.Hierarchy.push_back(ourNode);

				if (aParentIndex != -1)
				{
					outModelData.Hierarchy[thisIndex].Parent = aParentIndex;
					outModelData.Hierarchy[aParentIndex].Children.push_back(thisIndex);
				}

				for (unsigned i = 0; i < aNode->mNumChildren; ++i)
				{
					TraverseNode(aNode->mChildren[i], thisIndex);
				}
			};

		if (scene->mRootNode)
		{
			TraverseNode(scene->mRootNode, -1);
		}

		return outModelData.IsValid();
	}
}
