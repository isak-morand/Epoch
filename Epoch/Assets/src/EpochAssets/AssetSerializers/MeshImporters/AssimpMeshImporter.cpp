#include "AssimpMeshImporter.h"
#include <unordered_set>
#include <CommonUtilities/Math/Transform.h>
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
		//aiProcess_FindInstances |
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

		//DataTypes::Skeleton skeleton;
		//ImportSkeleton(scene, skeleton);
		//std::unordered_map<std::string_view, uint32_t> boneNameToIndex;
		//for (size_t i = 0; i < skeleton.BoneNames.size(); ++i)
		//{
		//	boneNameToIndex[skeleton.BoneNames[i]] = static_cast<uint32_t>(i);
		//}

		std::vector<DataTypes::MeshData> meshDataList(scene->mNumMeshes);

		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			auto& meshData = meshDataList[i];

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

			meshData.SubMeshes.push_back({ 0, (uint32_t)meshData.Indices.size() });
		}

		if (aImportSettings.FlattenHierarchy)
		{
			DataTypes::MeshData combined;
			uint32_t vertexOffset = 0;
			uint32_t indexOffset = 0;

			for (size_t i = 0; i < meshDataList.size(); ++i)
			{
				auto& src = meshDataList[i];

				for (auto& v : src.Vertices)
				{
					combined.Vertices.push_back(v);
				}

				for (auto idx : src.Indices)
				{
					combined.Indices.push_back(idx + vertexOffset);
				}

				combined.SubMeshes.push_back({ indexOffset, (uint32_t)src.Indices.size() });

				vertexOffset += (uint32_t)src.Vertices.size();
				indexOffset += (uint32_t)src.Indices.size();
			}

			if (combined.IsValid())
			{
				UUID meshUUID = Utils::GenerateSubAssetHandle(aMetadata.Handle, "Mesh", 0);
				auto meshAsset = std::make_shared<MeshAsset>(meshUUID);
				SetMeshData(meshAsset, combined);
				AssetManager::GetEditorAssetManager()->AddSubAsset(aMetadata.Handle, meshAsset, aMetadata.FilePath.stem().string());
				outModelData.MeshAssets.push_back(meshUUID);

				DataTypes::ModelData::Node rootNode;
				rootNode.Name = aMetadata.FilePath.stem().string();
				rootNode.MeshIndex = 0;
				rootNode.LocalTransform = CU::Matrix4x4f::Identity;
				outModelData.Hierarchy.push_back(rootNode);
			}
		}
		else
		{
			for (size_t i = 0; i < meshDataList.size(); ++i)
			{
				auto& meshData = meshDataList[i];
				if (!meshData.IsValid()) continue;

				UUID meshUUID = Utils::GenerateSubAssetHandle(aMetadata.Handle, "Mesh", (uint32_t)i);
				auto meshAsset = std::make_shared<MeshAsset>(meshUUID);
				SetMeshData(meshAsset, meshData);
				AssetManager::GetEditorAssetManager()->AddSubAsset(aMetadata.Handle, meshAsset, scene->mMeshes[i]->mName.C_Str());
				outModelData.MeshAssets.push_back(meshUUID);
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
		}

		return outModelData.IsValid();
	}

	bool AssimpMeshImporter::ImportSkeleton(const aiScene* scene, DataTypes::Skeleton& outSkeleton)
	{
		if (!scene || !scene->mRootNode)
		{
			return false;
		}

		std::unordered_set<std::string_view> boneNamesSet;

		for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
		{
			const aiMesh* mesh = scene->mMeshes[m];
			for (uint32_t b = 0; b < mesh->mNumBones; ++b)
			{
				boneNamesSet.insert(mesh->mBones[b]->mName.C_Str());
			}
		}

		if (boneNamesSet.empty())
		{
			return false;
		}

		struct BoneNode
		{
			std::string Name;
			int ParentIndex = -1;
			CU::Matrix4x4f LocalTransform;
		};

		std::vector<BoneNode> bones;
		std::unordered_map<std::string, uint32_t> nameToIndex;

		std::function<void(aiNode*, int)> TraverseNodes;
		TraverseNodes = [&](aiNode* node, int parentBoneIndex)
			{
				std::string nodeName = node->mName.C_Str();

				int thisBoneIndex = -1;
				bool isBone = boneNamesSet.count(nodeName) > 0;

				if (isBone)
				{
					BoneNode bone;
					bone.Name = nodeName;
					bone.ParentIndex = parentBoneIndex;
					bone.LocalTransform = Utils::FromAIMat4(node->mTransformation);

					thisBoneIndex = static_cast<int>(bones.size());
					bones.push_back(bone);
					nameToIndex[nodeName] = thisBoneIndex;
				}

				for (uint32_t i = 0; i < node->mNumChildren; ++i)
				{
					TraverseNodes(node->mChildren[i], thisBoneIndex);
				}
			};

		TraverseNodes(scene->mRootNode, -1);

		if (bones.empty())
		{
			return false;
		}

		size_t boneCount = bones.size();
		outSkeleton.BoneNames.resize(boneCount);
		outSkeleton.ParentIndices.resize(boneCount);
		outSkeleton.RestPose.resize(boneCount);
		outSkeleton.BoneToWorldPose.resize(boneCount);

		for (size_t i = 0; i < boneCount; ++i)
		{
			const auto& bone = bones[i];

			outSkeleton.BoneNames[i] = bone.Name;
			outSkeleton.ParentIndices[i] = bone.ParentIndex;

			DataTypes::BoneTransform& rest = outSkeleton.RestPose[i];
			bone.LocalTransform.Decompose(rest.Translation, rest.Rotation, rest.Scale);
		}

		for (size_t i = 0; i < boneCount; ++i)
		{
			CU::Matrix4x4f localMatrix = CU::Transform(outSkeleton.RestPose[i].Translation, outSkeleton.RestPose[i].Rotation, outSkeleton.RestPose[i].Scale).GetMatrix();

			int parent = outSkeleton.ParentIndices[i];
			if (parent >= 0)
			{
				outSkeleton.BoneToWorldPose[i] = localMatrix * outSkeleton.BoneToWorldPose[parent];
			}
			else
			{
				outSkeleton.BoneToWorldPose[i] = localMatrix;
			}
		}

		return outSkeleton.IsValid();
	}
}
