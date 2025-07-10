#pragma once
#include <EpochDataTypes/MeshData.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Epoch::Rendering
{
	struct MeshSpecification
	{
		std::vector<DataTypes::Vertex> Vertices;
		std::vector<DataTypes::Index> Indices;

		std::vector<DataTypes::MeshData::SubMesh> Submeshes;

		std::string DebugName;
	};

	class Mesh
	{
	public:
		Mesh() = delete;
		Mesh(std::string_view aDebugName, const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices);
		Mesh(std::string_view aDebugName, const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices, const std::vector<DataTypes::MeshData::SubMesh>& aSubMeshes);
		~Mesh() = default;

		std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return myVertexBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return myIndexBuffer; }

		const std::vector<DataTypes::MeshData::SubMesh>& GetSubMeshes() const { return mySubMeshes; }

	private:
		void CreateVertexAndIndexBuffers(const std::vector<DataTypes::Vertex>& aVertices, const std::vector<DataTypes::Index>& aIndices);

	private:
		std::string myDebugName;

		std::shared_ptr<VertexBuffer> myVertexBuffer;
		std::shared_ptr<IndexBuffer> myIndexBuffer;

		std::vector<DataTypes::MeshData::SubMesh> mySubMeshes;
	};
}
