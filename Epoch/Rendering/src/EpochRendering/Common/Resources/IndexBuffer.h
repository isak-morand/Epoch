#pragma once
#include <memory>
#include <EpochCore/Buffer.h>
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	struct IndexBufferSpecification
	{
		size_t SizeInBytes = 0;
		bool CpuWritable = false;
		Core::Buffer InitialData;
		std::string DebugName = "IndexBuffer";
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = delete;
		IndexBuffer(const IndexBufferSpecification& aSpecification);
		~IndexBuffer();

		nvrhi::BufferHandle GetHandle() const { return myHandle; }

		void SetData(Core::Buffer aBuffer, uint64_t aOffset = 0);

	private:
		nvrhi::BufferHandle myHandle;

		IndexBufferSpecification mySpecification;
	};
}
