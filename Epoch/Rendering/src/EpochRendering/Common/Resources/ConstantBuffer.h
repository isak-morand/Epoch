#pragma once
#include <memory>
#include <EpochCore/Buffer.h>
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	struct ConstantBufferSpecification
	{
		size_t SizeInBytes = 0;
		Core::Buffer InitialData;
		std::string DebugName = "ConstantBuffer";
	};

	class ConstantBuffer
	{
	public:
		ConstantBuffer() = delete;
		ConstantBuffer(const ConstantBufferSpecification& aSpecification);
		~ConstantBuffer();

		nvrhi::BufferHandle GetHandle() const { return myHandle; }

		void SetData(Core::Buffer aBuffer, uint64_t aOffset = 0);

	private:
		nvrhi::BufferHandle myHandle;

		ConstantBufferSpecification mySpecification;
	};
}
