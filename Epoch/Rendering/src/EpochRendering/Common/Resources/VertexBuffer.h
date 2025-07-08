#pragma once
#include <memory>
#include <EpochCore/Buffer.h>
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	struct VertexBufferSpecification
	{
		size_t SizeInBytes = 0;
		uint32_t VertexStride = 0;
		bool CpuWritable = false;
		Core::Buffer InitialData;
		std::string DebugName = "VertexBuffer";
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() = delete;
		VertexBuffer(const VertexBufferSpecification& aSpecification);
		~VertexBuffer();

		nvrhi::BufferHandle GetHandle() const { return myHandle; }

		void SetData(Core::Buffer aBuffer, uint64_t aOffset = 0);

	private:
		nvrhi::BufferHandle myHandle;

		VertexBufferSpecification mySpecification;
	};
}
