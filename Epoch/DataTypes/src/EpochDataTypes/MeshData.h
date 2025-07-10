#pragma once
#include <vector>
#include "Vertex.h"

namespace Epoch::DataTypes
{
	typedef uint32_t Index;

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<Index> Indices;
		
		struct SubMesh
		{
			uint32_t IndexOffset;
			uint32_t IndexCount;
		};
		
		std::vector<SubMesh> SubMeshes;

		bool IsValid() const { return !Vertices.empty() && !Indices.empty(); }
	};
}
