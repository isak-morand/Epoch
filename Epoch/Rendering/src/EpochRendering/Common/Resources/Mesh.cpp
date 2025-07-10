#include "epch.h"
#include "Mesh.h"
#include <EpochCore/Assert.h>

namespace Epoch::Rendering
{
	Mesh::Mesh(std::string_view aDebugName, const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices)
	{
		myDebugName = aDebugName;
		CreateVertexAndIndexBuffers(aVertices, aIndices);

		auto& submesh = mySubMeshes.emplace_back();
		submesh.IndexCount = (uint32_t)aIndices.size();
	}

	Mesh::Mesh(std::string_view aDebugName, const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices, const std::vector<DataTypes::MeshData::SubMesh>& aSubMeshes)
	{
		myDebugName = aDebugName;
		CreateVertexAndIndexBuffers(aVertices, aIndices);

		EPOCH_ASSERT(!aSubMeshes.empty(), "Failed to create mesh! A mesh needs atleast one sub-mesh");
		mySubMeshes = aSubMeshes;
	}

	void Mesh::CreateVertexAndIndexBuffers(const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices)
	{
		//Vertex buffer
		{
			VertexBufferSpecification vertexBufferSpecification;
			vertexBufferSpecification.DebugName = std::format("{}_VB", myDebugName);
			vertexBufferSpecification.VertexStride = sizeof(DataTypes::Vertex);
			vertexBufferSpecification.SizeInBytes = aVertices.size() * vertexBufferSpecification.VertexStride;
			vertexBufferSpecification.InitialData = Core::Buffer::Copy({ (void*)aVertices.data(), vertexBufferSpecification.SizeInBytes });

			myVertexBuffer = std::make_shared<VertexBuffer>(vertexBufferSpecification);
		}

		//Index buffer
		{
			IndexBufferSpecification indexBufferSpecification;
			indexBufferSpecification.DebugName = std::format("{}_IB", myDebugName);
			indexBufferSpecification.SizeInBytes = aIndices.size() * sizeof(uint32_t);
			indexBufferSpecification.InitialData = Core::Buffer::Copy({ (void*)aIndices.data(), indexBufferSpecification.SizeInBytes });

			myIndexBuffer = std::make_shared<IndexBuffer>(indexBufferSpecification);
		}
	}
}
